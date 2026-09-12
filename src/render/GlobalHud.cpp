#include "render/GlobalHud.h"
#include "render/AssetManager.h"
#include "render/SoundManager.h"
#include "render/Account.h"
#include "render/SceneManager.h"
#include <cstdlib>

namespace {
constexpr float WW = 1280.f;
constexpr float WH = 800.f;
constexpr float BTN = 40.f;          // 左上角键尺寸
constexpr float PAD = 48.f;          // 键间距

// 弹窗几何(加高: 440 -> 560)
constexpr float PW = 680.f;
constexpr float PH = 560.f;
constexpr float PL = (WW - PW) / 2.f;   // 300
constexpr float PT = (WH - PH) / 2.f;   // 120

// 主菜单页布局(音量行)
constexpr float ROW_Y = PT + 200.f;          // 音量行中心 y
constexpr float VOL_X = PL + 64.f;           // soundSetting 图标中心 x
constexpr float TRACK_L = PL + 150.f;        // 滑轨左端
constexpr float TRACK_W = 410.f;             // 滑轨长度
constexpr float KNOB = 34.f;                 // slider 显示尺寸

// 开发者模式弹窗布局(与主菜单同尺寸)
constexpr float DEV_BTN_Y = PT + 250.f;           // 未启用: [启用开发者模式] 按钮中心 y
constexpr float DEV_BTN_Y_ON = PT + PH - 74.f;    // 已启用: [关闭开发者模式] 按钮下移居中(不压滑条)
constexpr float DEV_INPUT_Y = PT + 165.f;         // 输入框中心 y
constexpr float DEV_INPUT_W = 380.f;
constexpr float DEV_INPUT_H = 54.f;
constexpr float DEV_ROW_Y = PT + 280.f;           // dev 滑条行中心 y
constexpr float DEV_MAX = 100000.f;               // 可调筹码上限

// 游戏规则速览(干练,按《项目游戏规则.docx》+ 现行机制总结; 每行 <= 30 字)
const char* RULES_TEXT =
    "【游戏目标】9 张手牌分成头/中/尾三道，逐道比大小，赢得越多越好。\n"
    "【牌型大小】豹子>同花顺>金花>顺子>对子>散牌。\n"
    "【特殊牌型】异色 2-3-5 可赢豹子；大王可变任意红牌，小王可变任意黑牌。\n"
    "【下注】每局每人下注金 1 份，总池=人数x注金，均分三小池。\n"
    "【对局流程】下注->发 9 张->限时 25 秒分三道并交牌。\n"
    "【比牌结算】三道依次比牌，胜者取对应小池，打满轮次结算本场。\n"
    "【入场资格】余额不足房间第一局注金时，不可进入该房间。\n"
    "【筹码显示】右上角筹码条图标随余额档位变化；每局牌背三色随机。\n"
    "【逃跑】一局结算后可逃跑提前结束本场，罚 100 筹码。\n"
    "【踢出】每局后余额不足下一局注金将被踢出本场（不扣费）。\n"
    "【破产】余额低于 100 判定破产，返回大厅时自动补足至 500。";
}

GlobalHud::GlobalHud(SceneManager* mgr) : mgr_(mgr) {
    AssetManager& am = AssetManager::instance();

    // ---- 左上角四键 ----
    btnMenu_.setTexture(am.icon("menuList"));
    btnMenu_.setPosition(sf::Vector2f(12.f, 12.f));
    btnMenu_.setSize(BTN);
    btnMenu_.setCallback([this]() { openPopup(); });

    btnMusic_.setTexture(am.icon("musicOn"));
    btnMusic_.setPosition(sf::Vector2f(12.f + PAD, 12.f));
    btnMusic_.setSize(BTN);
    btnMusic_.setCallback([this]() {
        SoundManager::instance().toggleBgm();   // 只开关背景音乐,音效不受影响
        btnMusic_.setTexture(AssetManager::instance().icon(
            SoundManager::instance().bgmOn() ? "musicOn" : "musicOff"));
    });

    btnWrench_.setTexture(am.icon("wrench"));
    btnWrench_.setPosition(sf::Vector2f(12.f + PAD * 2.f, 12.f));
    btnWrench_.setSize(BTN);
    btnWrench_.setCallback([this]() {
        // 组牌界面禁用开发者模式: 播 error 音效, 不弹窗
        if (mgr_ && mgr_->currentId() == SceneId::Arrange) {
            SoundManager::instance().playError();
            return;
        }
        openDevPopup();
    });

    btnHome_.setTexture(am.icon("home"));
    btnHome_.setPosition(sf::Vector2f(12.f + PAD * 3.f, 12.f));
    btnHome_.setSize(BTN);
    btnHome_.setCallback([this]() { if (mgr_) mgr_->onHomePressed(); });

    // ---- 弹窗 ----
    overlay_.setSize(sf::Vector2f(WW, WH));
    overlay_.setFillColor(sf::Color(0, 0, 0, 160));
    panel_.setSize(sf::Vector2f(PW, PH));
    panel_.setPosition(sf::Vector2f(PL, PT));
    panel_.setFillColor(sf::Color(30, 40, 70));
    panel_.setOutlineColor(sf::Color(255, 215, 0));
    panel_.setOutlineThickness(3.f);

    btnClose_.setTexture(am.icon("close"));
    btnClose_.setSize(40.f);
    btnClose_.setPosition(sf::Vector2f(PL + PW - 60.f, PT + 20.f));
    btnClose_.setCallback([this]() { closePopup(); });

    // ---- 主菜单页: 音量行 ----
    btnVolIcon_.setTexture(am.icon("soundSetting"));
    btnVolIcon_.setSize(46.f);
    btnVolIcon_.setPosition(sf::Vector2f(VOL_X - 23.f, ROW_Y - 23.f));
    btnVolIcon_.setCallback([this]() { toggleMute(); });

    trackRect_ = sf::FloatRect(TRACK_L, ROW_Y - 5.f, TRACK_W, 10.f);
    track_.setSize(sf::Vector2f(TRACK_W, 10.f));
    track_.setPosition(sf::Vector2f(TRACK_L, ROW_Y - 5.f));
    track_.setFillColor(sf::Color(46, 58, 96));
    track_.setOutlineColor(sf::Color(120, 140, 190));
    track_.setOutlineThickness(1.f);
    trackFill_.setSize(sf::Vector2f(0.f, 10.f));
    trackFill_.setPosition(sf::Vector2f(TRACK_L, ROW_Y - 5.f));
    trackFill_.setFillColor(sf::Color(255, 215, 0));

    knob_.setTexture(am.icon("slider"));
    knob_.setSize(KNOB);

    btnRules_.setText("游戏规则");
    btnRules_.setPosition(sf::Vector2f(PL + 100.f, PT + PH - 130.f));
    btnRules_.setSize(sf::Vector2f(PW - 200.f, 56.f));
    btnRules_.setCallback([this]() { showRulesPage(); });

    // ---- 规则页 ----
    rulesTitle_.setText("游戏规则");
    rulesTitle_.setCharacterSize(30);
    rulesTitle_.setColor(sf::Color(255, 220, 130));
    rulesTitle_.centerOrigin();
    rulesTitle_.setPosition(sf::Vector2f(WW / 2.f, PT + 52.f));

    rulesText_.setText(RULES_TEXT);
    rulesText_.setCharacterSize(19);
    rulesText_.setColor(sf::Color(230, 230, 230));
    rulesText_.setPosition(sf::Vector2f(PL + 44.f, PT + 110.f));

    btnRulesBack_.setText("返回");
    btnRulesBack_.setPosition(sf::Vector2f(WW / 2.f - 80.f, PT + PH - 96.f));
    btnRulesBack_.setSize(sf::Vector2f(160.f, 48.f));
    btnRulesBack_.setCallback([this]() { showMainPage(); });

    // ---- 开发者模式弹窗 ----
    btnDevToggle_.setText("启用开发者模式");
    btnDevToggle_.setPosition(sf::Vector2f(WW / 2.f - 190.f, DEV_BTN_Y - 30.f));
    btnDevToggle_.setSize(sf::Vector2f(380.f, 60.f));
    btnDevToggle_.setCallback([this]() {
        if (devOn_) {
            // 已启用: 点一下即关闭
            devOn_ = false;
            devArm_ = false;
            devInputFocus_ = false;
            btnDevToggle_.setText("启用开发者模式");
            btnDevToggle_.setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
            btnDevToggle_.setPosition(sf::Vector2f(WW / 2.f - 190.f, DEV_BTN_Y - 30.f));
        } else {
            // 两段确认: 第一次进入确认态, 5 秒内再点一次才真正启用
            if (!devArm_) {
                devArm_ = true;
                devArmTimer_ = 0.f;
                btnDevToggle_.setText("再点一次确认启用");
                btnDevToggle_.setColors(sf::Color(200, 90, 60), sf::Color(230, 120, 90), sf::Color(160, 60, 40));
            } else {
                devArm_ = false;
                devOn_ = true;
                devInputStr_.clear();
                devInputFocus_ = false;
                btnDevToggle_.setText("关闭开发者模式");
                btnDevToggle_.setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
                btnDevToggle_.setPosition(sf::Vector2f(WW / 2.f - 190.f, DEV_BTN_Y_ON - 30.f));   // 下移居中
            }
        }
    });

    // dev 滑条(0 ~ 100000)
    devTrackRect_ = sf::FloatRect(TRACK_L, DEV_ROW_Y - 5.f, TRACK_W, 10.f);
    devTrack_.setSize(sf::Vector2f(TRACK_W, 10.f));
    devTrack_.setPosition(sf::Vector2f(TRACK_L, DEV_ROW_Y - 5.f));
    devTrack_.setFillColor(sf::Color(46, 58, 96));
    devTrack_.setOutlineColor(sf::Color(120, 140, 190));
    devTrack_.setOutlineThickness(1.f);
    devFill_.setSize(sf::Vector2f(0.f, 10.f));
    devFill_.setPosition(sf::Vector2f(TRACK_L, DEV_ROW_Y - 5.f));
    devFill_.setFillColor(sf::Color(255, 215, 0));
    devKnob_.setTexture(am.icon("slider"));
    devKnob_.setSize(KNOB);

    // 输入框
    devInputBox_.setSize(sf::Vector2f(DEV_INPUT_W, DEV_INPUT_H));
    devInputBox_.setPosition(sf::Vector2f(WW / 2.f - DEV_INPUT_W / 2.f, DEV_INPUT_Y - DEV_INPUT_H / 2.f));
    devInputBox_.setFillColor(sf::Color(20, 26, 46));
    devInputBox_.setOutlineColor(sf::Color(140, 160, 210));
    devInputBox_.setOutlineThickness(2.f);

    // 初始音量
    vol_ = static_cast<float>(SoundManager::instance().volume());
    savedVol_ = vol_;
    updateKnob();
}

bool GlobalHud::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (mgr_) btnHome_.setVisible(mgr_->homeVisible());

    // dev 弹窗优先于菜单弹窗
    bool devPopup = devPopupOpen_;
    if (!devPopup && !popupOpen_) {
        // 常态: 四键 -> 场景
        btnMenu_.handleEvent(e, win);
        btnMusic_.handleEvent(e, win);
        btnWrench_.handleEvent(e, win);
        btnHome_.handleEvent(e, win);
        return false;
    }

    // ---- 弹窗打开: 拦截 ----
    if (devPopup) {
        // ---- 开发者模式弹窗 ----
        // 键盘输入(启用后且输入框聚焦)
        if (devOn_ && devInputFocus_) {
            handleDevText(e);
        }
        if (e.type == sf::Event::MouseButtonPressed &&
            e.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mp = win.mapPixelToCoords(sf::Vector2i(e.mouseButton.x,
                                                                e.mouseButton.y));
            // 输入框聚焦判定
            sf::FloatRect ib(devInputBox_.getPosition(), devInputBox_.getSize());
            devInputFocus_ = ib.contains(mp);
            if (devInputFocus_ && devInputStr_.empty()) {
                devInputStr_ = std::to_string(Account::instance().balance());
            }
            if (!devInputFocus_ && devInputStr_.empty()) {
                devInputStr_.clear();
            }
            // dev 滑条拖动
            if (devOn_) {
                float hw = KNOB / 2.f + 8.f;
                sf::FloatRect hit(devTrackRect_.left - hw, DEV_ROW_Y - hw,
                                  devTrackRect_.width + hw * 2.f, hw * 2.f);
                if (hit.contains(mp)) {
                    devDrag_ = true;
                    setBalanceFromMouse(mp.x);
                }
            }
        } else if (e.type == sf::Event::MouseButtonReleased &&
                   e.mouseButton.button == sf::Mouse::Left) {
            devDrag_ = false;
        } else if (e.type == sf::Event::MouseMoved && devDrag_ && devOn_) {
            sf::Vector2f mp = win.mapPixelToCoords(sf::Vector2i(e.mouseMove.x,
                                                                e.mouseMove.y));
            setBalanceFromMouse(mp.x);
        }
        btnClose_.handleEvent(e, win);
        btnDevToggle_.handleEvent(e, win);
        return true;
    }

    // ---- 菜单弹窗 ----
    if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mp = win.mapPixelToCoords(sf::Vector2i(e.mouseButton.x,
                                                            e.mouseButton.y));
        if (!showRules_) {
            float hw = KNOB / 2.f + 8.f;
            sf::FloatRect hit(trackRect_.left - hw, ROW_Y - hw,
                              trackRect_.width + hw * 2.f, hw * 2.f);
            if (hit.contains(mp)) {
                dragging_ = true;
                setVolumeFromMouse(mp.x);
            }
        }
    } else if (e.type == sf::Event::MouseButtonReleased &&
               e.mouseButton.button == sf::Mouse::Left) {
        dragging_ = false;
    } else if (e.type == sf::Event::MouseMoved && dragging_ && !showRules_) {
        sf::Vector2f mp = win.mapPixelToCoords(sf::Vector2i(e.mouseMove.x,
                                                            e.mouseMove.y));
        setVolumeFromMouse(mp.x);
    }
    btnClose_.handleEvent(e, win);
    if (showRules_) {
        btnRulesBack_.handleEvent(e, win);
    } else {
        btnVolIcon_.handleEvent(e, win);
        btnRules_.handleEvent(e, win);
    }
    return true;
}

void GlobalHud::update(float dt) {
    // 两段确认 5 秒未二次点击则复原
    if (devArm_) {
        devArmTimer_ += dt;
        if (devArmTimer_ >= 5.f) {
            devArm_ = false;
            btnDevToggle_.setText("启用开发者模式");
            btnDevToggle_.setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
        }
    }
}

void GlobalHud::draw(sf::RenderWindow& win) {
    if (mgr_) btnHome_.setVisible(mgr_->homeVisible());
    btnMenu_.draw(win);
    btnMusic_.draw(win);
    btnWrench_.draw(win);
    btnHome_.draw(win);

    if (!devPopupOpen_ && !popupOpen_) return;
    // 弹窗层
    win.draw(overlay_);
    win.draw(panel_);
    sf::CircleShape ring(24.f);
    ring.setPosition(sf::Vector2f(PL + PW - 60.f + 20.f - 24.f,
                                  PT + 20.f + 20.f - 24.f));
    ring.setFillColor(sf::Color(255, 215, 0, 30));
    ring.setOutlineColor(sf::Color(255, 215, 0));
    ring.setOutlineThickness(3.f);
    win.draw(ring);
    btnClose_.draw(win);

    if (devPopupOpen_) {
        drawDevPopup(win);
        return;
    }
    if (showRules_) {
        rulesTitle_.draw(win);
        rulesText_.draw(win);
        btnRulesBack_.draw(win);
        return;
    }
    // 主菜单页
    TextBox ttl("菜单", sf::Vector2f(WW / 2.f, PT + 52.f), 28);
    ttl.setColor(sf::Color(255, 220, 130));
    ttl.centerOrigin();
    ttl.draw(win);
    btnVolIcon_.setTexture(AssetManager::instance().icon(
        vol_ <= 0.f ? "soundOff" : "soundSetting"));
    btnVolIcon_.draw(win);
    win.draw(track_);
    trackFill_.setSize(sf::Vector2f(vol_ / 100.f * TRACK_W, 10.f));
    win.draw(trackFill_);
    knob_.draw(win);
    TextBox tip("拖动滑块或点击左侧图标调节所有声音大小",
                sf::Vector2f(TRACK_L + TRACK_W / 2.f, ROW_Y + 62.f), 17);
    tip.setColor(sf::Color(190, 190, 190));
    tip.centerOrigin();
    tip.draw(win);
    btnRules_.draw(win);
}

// 开发者模式弹窗绘制(与主菜单弹窗同尺寸)
void GlobalHud::drawDevPopup(sf::RenderWindow& win) {
    if (!devOn_) {
        // ---- 未启用: 提示 + 启用按钮 ----
        TextBox ttl("工具", sf::Vector2f(WW / 2.f, PT + 52.f), 28);
        ttl.setColor(sf::Color(255, 220, 130));
        ttl.centerOrigin();
        ttl.draw(win);
        TextBox desc("开发者模式：可随时修改筹码余额，方便测试不同人数/注金房间。",
                     sf::Vector2f(WW / 2.f, PT + 140.f), 17);
        desc.setColor(sf::Color(200, 200, 200));
        desc.centerOrigin();
        desc.draw(win);
        TextBox note("启用后每次启动游戏需重新开启。", sf::Vector2f(WW / 2.f, PT + 178.f), 17);
        note.setColor(sf::Color(150, 160, 190));
        note.centerOrigin();
        note.draw(win);
        btnDevToggle_.draw(win);
        return;
    }
    // ---- 已启用: 标注 + 输入框 + 筹码+滑条 + 关闭按钮 ----
    TextBox ttl("开发者模式", sf::Vector2f(WW / 2.f, PT + 44.f), 28);
    ttl.setColor(sf::Color(255, 220, 130));
    ttl.centerOrigin();
    ttl.draw(win);
    TextBox badge("（已启用开发者模式）", sf::Vector2f(WW / 2.f, PT + 84.f), 20);
    badge.setColor(sf::Color(120, 255, 160));
    badge.centerOrigin();
    badge.draw(win);

    // 输入框(聚焦显示编辑串; 未聚焦显示当前余额)
    win.draw(devInputBox_);
    TextBox label("直接输入筹码数（0 ~ 100000）", sf::Vector2f(WW / 2.f, DEV_INPUT_Y - DEV_INPUT_H / 2.f - 18.f), 16);
    label.setColor(sf::Color(180, 190, 220));
    label.centerOrigin();
    label.draw(win);
    std::string shown = devInputFocus_ ? devInputStr_ : std::to_string(Account::instance().balance());
    if (devInputFocus_) shown += "|";   // 简易光标
    TextBox inputText(shown, sf::Vector2f(WW / 2.f, DEV_INPUT_Y), 24);
    inputText.setColor(sf::Color(255, 255, 255));
    inputText.centerOrigin();
    inputText.draw(win);

    // 筹码图标 + 滑条(0~100000)
    int bal = Account::instance().balance();
    if (const sf::Texture* ct = AssetManager::instance().chipForAmount(bal)) {
        sf::Sprite chip(*ct);
        float cs = 46.f / static_cast<float>(ct->getSize().x);
        chip.setScale(cs, cs);
        chip.setPosition(sf::Vector2f(VOL_X - 23.f, DEV_ROW_Y - 23.f));
        win.draw(chip);
    }
    win.draw(devTrack_);
    float t = (bal > DEV_MAX ? DEV_MAX : (bal < 0 ? 0.f : static_cast<float>(bal))) / DEV_MAX;
    devFill_.setSize(sf::Vector2f(t * TRACK_W, 10.f));
    win.draw(devFill_);
    updateDevKnob();
    devKnob_.draw(win);
    TextBox tip("拖动滑条或输入数字直接修改余额（上限 100000）",
                sf::Vector2f(TRACK_L + TRACK_W / 2.f, DEV_ROW_Y + 62.f), 16);
    tip.setColor(sf::Color(190, 190, 190));
    tip.centerOrigin();
    tip.draw(win);
    btnDevToggle_.draw(win);
}

void GlobalHud::openPopup() {
    devPopupOpen_ = false;   // 互斥
    popupOpen_ = true;
    showRules_ = false;
}

void GlobalHud::openDevPopup() {
    popupOpen_ = false;      // 互斥
    devPopupOpen_ = true;
    devArm_ = false;
    devArmTimer_ = 0.f;
    devInputFocus_ = false;
    devInputStr_.clear();
    // 按当前启用状态同步按钮文字/位置(启用态按钮在下方居中)
    btnDevToggle_.setText(devOn_ ? "关闭开发者模式" : "启用开发者模式");
    btnDevToggle_.setPosition(sf::Vector2f(WW / 2.f - 190.f,
                                           (devOn_ ? DEV_BTN_Y_ON : DEV_BTN_Y) - 30.f));
}

void GlobalHud::closePopup() {
    if (devPopupOpen_) {
        devPopupOpen_ = false;
        devDrag_ = false;
        devInputFocus_ = false;
        devArm_ = false;
        return;
    }
    popupOpen_ = false;
    showRules_ = false;
    dragging_ = false;
}

void GlobalHud::showRulesPage() { showRules_ = true; dragging_ = false; }
void GlobalHud::showMainPage()  { showRules_ = false; }

void GlobalHud::updateKnob() {
    float x = TRACK_L + vol_ / 100.f * TRACK_W - KNOB / 2.f;
    knob_.setPosition(sf::Vector2f(x, ROW_Y - KNOB / 2.f));
}

void GlobalHud::setVolume(int v) {
    if (v < 0) v = 0;
    if (v > 100) v = 100;
    vol_ = static_cast<float>(v);
    SoundManager::instance().setVolume(v);
    updateKnob();
}

void GlobalHud::setVolumeFromMouse(float mx) {
    float t = (mx - TRACK_L) / TRACK_W;
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    setVolume(static_cast<int>(t * 100.f));
}

void GlobalHud::toggleMute() {
    if (vol_ > 0.f) {
        savedVol_ = vol_;
        setVolume(0);
    } else {
        int restore = static_cast<int>(savedVol_ > 0.f ? savedVol_ : 100.f);
        setVolume(restore);
    }
}

void GlobalHud::updateDevKnob() {
    int bal = Account::instance().balance();
    float t = (bal > DEV_MAX ? DEV_MAX : (bal < 0 ? 0.f : static_cast<float>(bal))) / DEV_MAX;
    float x = TRACK_L + t * TRACK_W - KNOB / 2.f;
    devKnob_.setPosition(sf::Vector2f(x, DEV_ROW_Y - KNOB / 2.f));
}

void GlobalHud::setBalanceFromMouse(float mx) {
    float t = (mx - TRACK_L) / TRACK_W;
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    int v = static_cast<int>(t * DEV_MAX);
    Account::instance().setBalance(v);
    if (devInputFocus_) devInputStr_ = std::to_string(v);  // 同步输入框显示
}

void GlobalHud::applyDevInput() {
    int v = 0;
    if (!devInputStr_.empty()) {
        v = std::atoi(devInputStr_.c_str());
    }
    if (v < 0) v = 0;
    if (v > static_cast<int>(DEV_MAX)) v = static_cast<int>(DEV_MAX);
    Account::instance().setBalance(v);
    devInputStr_ = std::to_string(v);
}

// 输入框键盘处理(数字/退格, 即输即改)
void GlobalHud::handleDevText(const sf::Event& e) {
    if (e.type == sf::Event::TextEntered) {
        sf::Uint32 c = e.text.unicode;
        if (c >= '0' && c <= '9') {
            if (devInputStr_.size() < 6) {   // 上限 100000(6 位)
                devInputStr_.push_back(static_cast<char>(c));
            }
            applyDevInput();
        } else if (c == 8) {                 // Backspace
            if (!devInputStr_.empty()) {
                devInputStr_.pop_back();
            }
            applyDevInput();
        }
    }
}
