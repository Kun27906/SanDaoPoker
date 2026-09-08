#include "render/GlobalHud.h"
#include "render/AssetManager.h"
#include "render/SoundManager.h"
#include "render/SceneManager.h"

namespace {
constexpr float WW = 1280.f;
constexpr float WH = 800.f;
constexpr float BTN = 40.f;          // 左上角键尺寸
constexpr float PAD = 48.f;          // 键间距

// 弹窗几何(加高: 440 -> 560, 容纳规则按钮/规则正文)
constexpr float PW = 680.f;
constexpr float PH = 560.f;
constexpr float PL = (WW - PW) / 2.f;   // 300
constexpr float PT = (WH - PH) / 2.f;   // 120

// 主菜单页布局
constexpr float ROW_Y = PT + 200.f;          // 音量行中心 y
constexpr float VOL_X = PL + 64.f;           // soundSetting 图标中心 x
constexpr float TRACK_L = PL + 150.f;        // 滑轨左端
constexpr float TRACK_W = 410.f;             // 滑轨长度
constexpr float KNOB = 34.f;                 // slider 显示尺寸

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
    // wrench: 占位,后续接入具体功能(点击已有 click 反馈)

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

    // 右上角关闭键(画圆: btnClose_ 下方衬一个圆底)
    btnClose_.setTexture(am.icon("close"));
    btnClose_.setSize(40.f);
    btnClose_.setPosition(sf::Vector2f(PL + PW - 60.f, PT + 20.f));
    btnClose_.setCallback([this]() { closePopup(); });

    // ---- 主菜单页: 音量行 ----
    btnVolIcon_.setTexture(am.icon("soundSetting"));
    btnVolIcon_.setSize(46.f);
    btnVolIcon_.setPosition(sf::Vector2f(VOL_X - 23.f, ROW_Y - 23.f));
    btnVolIcon_.setCallback([this]() { toggleMute(); });   // 点击图标 = 静音切换

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

    // ---- 主菜单页: 游戏规则按钮 ----
    btnRules_.setText("游戏规则");
    btnRules_.setPosition(sf::Vector2f(PL + 100.f, PT + PH - 130.f));
    btnRules_.setSize(sf::Vector2f(PW - 200.f, 56.f));
    btnRules_.setCallback([this]() { showRulesPage(); });

    // ---- 规则页: 标题/正文/返回 ----
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

    // 初始音量(默认 100,与系统音量 0~100 刻度一致)
    vol_ = static_cast<float>(SoundManager::instance().volume());
    savedVol_ = vol_;
    updateKnob();
}

bool GlobalHud::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    // home 键显隐随场景(组牌/比牌/启动页隐藏)
    if (mgr_) btnHome_.setVisible(mgr_->homeVisible());

    if (!popupOpen_) {
        // 常态: 四键 -> 场景
        btnMenu_.handleEvent(e, win);
        btnMusic_.handleEvent(e, win);
        btnWrench_.handleEvent(e, win);
        btnHome_.handleEvent(e, win);
        return false;
    }

    // ---- 弹窗打开: 拦截一切,只处理弹窗 ----
    if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left) {
        // 事件坐标 -> 渲染逻辑坐标(高 DPI 下必须映射,否则点击错位)
        sf::Vector2f mp = win.mapPixelToCoords(sf::Vector2i(e.mouseButton.x,
                                                            e.mouseButton.y));
        if (showRules_) {
            // 规则页: 点击不作用于滑轨(轨道只属于主菜单页)
        } else {
            // 主菜单页: 点击滑轨/滑块 -> 开始拖动(并跳转到该位置)
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
        btnVolIcon_.handleEvent(e, win);   // 点音量图标 = 静音切换
        btnRules_.handleEvent(e, win);
    }
    return true;   // 事件已消耗
}

void GlobalHud::draw(sf::RenderWindow& win) {
    if (mgr_) btnHome_.setVisible(mgr_->homeVisible());
    btnMenu_.draw(win);
    btnMusic_.draw(win);
    btnWrench_.draw(win);
    btnHome_.draw(win);
    if (!popupOpen_) return;

    // 弹窗层
    win.draw(overlay_);
    win.draw(panel_);

    // 右上角圆圈 + 关闭
    sf::CircleShape ring(24.f);
    ring.setPosition(sf::Vector2f(PL + PW - 60.f + 20.f - 24.f,
                                  PT + 20.f + 20.f - 24.f));
    ring.setFillColor(sf::Color(255, 215, 0, 30));
    ring.setOutlineColor(sf::Color(255, 215, 0));
    ring.setOutlineThickness(3.f);
    win.draw(ring);
    btnClose_.draw(win);

    if (showRules_) {
        // ---- 规则页 ----
        rulesTitle_.draw(win);
        rulesText_.draw(win);
        btnRulesBack_.draw(win);
        return;
    }

    // ---- 主菜单页 ----
    TextBox ttl("菜单", sf::Vector2f(WW / 2.f, PT + 52.f), 28);
    ttl.setColor(sf::Color(255, 220, 130));
    ttl.centerOrigin();
    ttl.draw(win);

    // 音量行: 图标(0 音量 -> soundOff) + 滑轨(金色已填充) + slider 滑块
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

void GlobalHud::openPopup() {
    popupOpen_ = true;
    showRules_ = false;   // 每次打开回到主菜单页
}

void GlobalHud::closePopup() {
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
        savedVol_ = vol_;        // 记住当前音量
        setVolume(0);            // 静音:滑块滑到最左,图标自动变 soundOff
    } else {
        int restore = static_cast<int>(savedVol_ > 0.f ? savedVol_ : 100.f);
        setVolume(restore);      // 恢复
    }
}
