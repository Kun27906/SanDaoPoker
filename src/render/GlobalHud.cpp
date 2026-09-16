#include "render/GlobalHud.h"
#include "render/Layout.h"
#include "render/AssetManager.h"
#include "ui/PanelFrame.h"
#include "render/SoundManager.h"
#include "render/Account.h"
#include "render/SceneManager.h"
#include "ui/FontUtil.h"
#include <cstdlib>
#include <vector>

namespace {
constexpr float WW = static_cast<float>(layout::WINDOW_W);
constexpr float WH = static_cast<float>(layout::WINDOW_H);
constexpr float BTN = 40.f;
constexpr float PAD = 48.f;

constexpr float PW = 680.f;
constexpr float PH = 560.f;
constexpr float PL = (WW - PW) / 2.f;
constexpr float PT = (WH - PH) / 2.f;

constexpr float EXIT_W = 720.f;
constexpr float EXIT_H = 260.f;
constexpr float EXIT_L = (WW - EXIT_W) / 2.f;
constexpr float EXIT_T = (WH - EXIT_H) / 2.f;
constexpr float EXIT_TEXT_DY = 84.f;
constexpr float EXIT_BTN_DY = 78.f;
constexpr float EXIT_BTN_W = 180.f;
constexpr float EXIT_BTN_H = 52.f;

constexpr float ROW_Y = PT + 200.f;
constexpr float VOL_X = PL + 64.f;
constexpr float TRACK_L = PL + 150.f;
constexpr float TRACK_W = 410.f;
constexpr float KNOB = 34.f;

constexpr float DEV_BTN_Y = PT + 250.f;
constexpr float DEV_BTN_Y_ON = PT + PH - 74.f;
constexpr float DEV_INPUT_Y = PT + 165.f;
constexpr float DEV_INPUT_W = 380.f;
constexpr float DEV_INPUT_H = 54.f;
constexpr float DEV_ROW_Y = PT + 280.f;
constexpr float DEV_MAX = 100000.f;

// 规则页文本区与滚动条
constexpr unsigned RULES_FONT_SIZE = 19;
constexpr float RULES_X = PL + 44.f;
constexpr float RULES_Y = PT + 110.f;
constexpr float RULES_W = PW - 88.f;
constexpr float RULES_H = PH - 110.f - 104.f;
constexpr float RULES_WRAP_W = RULES_W - 24.f;
constexpr float RULES_SCROLL_STEP = 56.f;
constexpr float RULES_BAR_W = 6.f;
constexpr float RULES_BAR_X = PL + PW - 26.f;

const char* RULES_TEXT =
    "【游戏目标】9 张手牌分成头/中/尾三道，逐道比大小，赢得越多越好。\n"
    "【牌型大小】豹子>同花顺>金花>顺子>对子>散牌。\n"
    "【特殊牌型】异色 2-3-5 可赢豹子；大王可变任意红牌，小王可变任意黑牌。\n"
    "【下注】每局每人下注金 1 份，总池=人数x注金，均分三小池。\n"
    "【对局流程】下注->发 9 张->限时 25 秒分三道并交牌。\n"
    "【比牌结算】三道依次比牌，胜者取对应小池，打满轮次结算本场。\n"
    "【入场资格】余额不足房间第一局注金时，不可进入该房间。\n"
    "【筹码显示】右上角筹码条图标随余额档位变化；每局牌背三色随机。\n"
    "【逃跑】一局结算后可提前结束本场；局内直接关闭游戏窗口同样按逃跑结算。\n"
    "【逃跑罚金】一局都没打完不罚；1-2 局罚 1 份底注，3-4 局 2 份，5-8 局 3 份，9 局及以上 4 份。\n"
    "【踢出】每局后余额不足下一局注金将被踢出本场（不扣费）。\n"
    "【破产】余额低于 100 判定破产，返回大厅时自动补足至 500。";

// 中文折行: 按宽度断行, 收尾标点不落行首。ASCII 连续串视为一个词整体换行

std::vector<std::string> splitUtf8(const std::string& s) {
    std::vector<std::string> out;
    for (size_t i = 0; i < s.size();) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        size_t len = 1;
        if (c >= 0xF0) len = 4;
        else if (c >= 0xE0) len = 3;
        else if (c >= 0xC0) len = 2;
        if (i + len > s.size()) len = 1;
        out.push_back(s.substr(i, len));
        i += len;
    }
    return out;
}

bool isWordChar(char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') || c == '-' || c == '.';
}

bool isClosingPunct(const std::string& cp) {
    static const char* kSet[] = { "，", "。", "、", "；", "：", "！", "？", "）",
                                  "】", "》", "」", "』", "”", "’" };
    for (const char* s : kSet) {
        if (cp == s) return true;
    }
    if (cp.size() == 1) {
        const char c = cp[0];
        return c == ',' || c == '.' || c == ';' || c == ':' ||
               c == '!' || c == '?' || c == ')' || c == ']' || c == '%';
    }
    return false;
}

std::vector<std::string> wrapCjkText(const std::string& src, const sf::Font& font,
                                     unsigned size, float maxW) {
    sf::Text meas;
    meas.setFont(font);
    meas.setCharacterSize(size);
    std::vector<std::string> lines;
    std::string cur;

    auto widthOf = [&](const std::string& s) -> float {
        if (s.empty()) return 0.f;
        meas.setString(str_util::utf8(s.c_str()));
        return meas.getLocalBounds().width;
    };
    auto flush = [&]() {
        while (!cur.empty() && cur.back() == ' ') cur.pop_back();
        if (!cur.empty()) lines.push_back(cur);
        cur.clear();
    };

    size_t p = 0;
    while (p <= src.size()) {
        const size_t q = src.find('\n', p);
        const std::string para = src.substr(p, (q == std::string::npos) ? std::string::npos : q - p);

        // 分词: ASCII 连续串为一个词, 其余逐字符
        std::vector<std::string> tokens;
        std::string word;
        for (const std::string& cp : splitUtf8(para)) {
            if (cp.size() == 1 && isWordChar(cp[0])) {
                word += cp;
                continue;
            }
            if (!word.empty()) { tokens.push_back(word); word.clear(); }
            tokens.push_back(cp);
        }
        if (!word.empty()) tokens.push_back(word);

        for (const std::string& tk : tokens) {
            if (tk == " ") {
                if (!cur.empty() && widthOf(cur + tk) <= maxW) cur += tk;
                continue;
            }
            if (!cur.empty() && !isClosingPunct(tk) && widthOf(cur + tk) > maxW) {
                flush();
            }
            cur += tk;
            // 收尾标点允许悬挂, 但累计悬挂过多时收行
            if (widthOf(cur) > maxW + 20.f) flush();
        }
        flush();
        if (q == std::string::npos) break;
        p = q + 1;
    }
    return lines;
}

} // namespace

GlobalHud::GlobalHud(SceneManager* mgr) : mgr_(mgr) {
    AssetManager& am = AssetManager::instance();

    btnMenu_.setTexture(am.icon("menuList"));
    btnMenu_.setPosition(sf::Vector2f(12.f, 12.f));
    btnMenu_.setSize(BTN);
    btnMenu_.setCallback([this]() { openPopup(); });

    btnMusic_.setTexture(am.icon("musicOn"));
    btnMusic_.setPosition(sf::Vector2f(12.f + PAD, 12.f));
    btnMusic_.setSize(BTN);
    btnMusic_.setCallback([this]() {
        SoundManager::instance().toggleBgm();
        btnMusic_.setTexture(AssetManager::instance().icon(
            SoundManager::instance().bgmOn() ? "musicOn" : "musicOff"));
    });

    btnWrench_.setTexture(am.icon("wrench"));
    btnWrench_.setPosition(sf::Vector2f(12.f + PAD * 2.f, 12.f));
    btnWrench_.setSize(BTN);
    btnWrench_.setCallback([this]() {
        // 局内禁用开发者模式
        if (mgr_) {
            SceneId id = mgr_->currentId();
            if (id == SceneId::Deal || id == SceneId::Arrange || id == SceneId::Battle) {
                SoundManager::instance().playError();
                return;
            }
        }
        openDevPopup();
    });

    btnHome_.setTexture(am.icon("home"));
    btnHome_.setPosition(sf::Vector2f(12.f + PAD * 3.f, 12.f));
    btnHome_.setSize(BTN);
    btnHome_.setCallback([this]() { if (mgr_) mgr_->onHomePressed(); });

    exitDialog_.setSize(sf::Vector2f(EXIT_W, EXIT_H));
    exitDialog_.setPosition(sf::Vector2f(EXIT_L, EXIT_T));
    exitDialog_.setFillColor(sf::Color(30, 40, 70));
    exitDialog_.setOutlineColor(sf::Color(255, 215, 0));
    exitDialog_.setOutlineThickness(3.f);

    exitText_.setText("本局还未结束，您想要退出吗？\n如果退出，将按逃跑提前结算。");
    exitText_.setCharacterSize(22);
    exitText_.setColor(sf::Color(235, 235, 235));
    exitText_.centerOrigin();
    exitText_.setPosition(sf::Vector2f(WW / 2.f, EXIT_T + EXIT_TEXT_DY));

    btnExitOk_.setText("确定");
    btnExitOk_.setPosition(sf::Vector2f(WW / 2.f - EXIT_BTN_W - 15.f,
                                        EXIT_T + EXIT_H - EXIT_BTN_DY - EXIT_BTN_H / 2.f));
    btnExitOk_.setSize(sf::Vector2f(EXIT_BTN_W, EXIT_BTN_H));
    btnExitOk_.setCallback([this]() { confirmExit(); });

    btnExitCancel_.setText("取消");
    btnExitCancel_.setPosition(sf::Vector2f(WW / 2.f + 15.f,
                                            EXIT_T + EXIT_H - EXIT_BTN_DY - EXIT_BTN_H / 2.f));
    btnExitCancel_.setSize(sf::Vector2f(EXIT_BTN_W, EXIT_BTN_H));
    btnExitCancel_.setCallback([this]() { exitPopupOpen_ = false; });

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

    rulesTitle_.setText("游戏规则");
    rulesTitle_.setCharacterSize(30);
    rulesTitle_.setColor(sf::Color(255, 220, 130));
    rulesTitle_.centerOrigin();
    rulesTitle_.setPosition(sf::Vector2f(WW / 2.f, PT + 52.f));

    rebuildRulesText();

    btnRulesBack_.setText("返回");
    btnRulesBack_.setPosition(sf::Vector2f(WW / 2.f - 80.f, PT + PH - 96.f));
    btnRulesBack_.setSize(sf::Vector2f(160.f, 48.f));
    btnRulesBack_.setCallback([this]() { showMainPage(); });

    btnDevToggle_.setText("启用开发者模式");
    btnDevToggle_.setPosition(sf::Vector2f(WW / 2.f - 190.f, DEV_BTN_Y - 30.f));
    btnDevToggle_.setSize(sf::Vector2f(380.f, 60.f));
    btnDevToggle_.setCallback([this]() {
        if (devOn_) {
            devOn_ = false;
            devArm_ = false;
            devInputFocus_ = false;
        } else if (!devArm_) {
            // 两段确认, 5 秒内再点一次才启用
            devArm_ = true;
            devArmTimer_ = 0.f;
        } else {
            devArm_ = false;
            devOn_ = true;
            devInputStr_.clear();
            devInputFocus_ = false;
        }
        refreshDevToggle();
    });

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

    devInputBox_.setSize(sf::Vector2f(DEV_INPUT_W, DEV_INPUT_H));
    devInputBox_.setPosition(sf::Vector2f(WW / 2.f - DEV_INPUT_W / 2.f, DEV_INPUT_Y - DEV_INPUT_H / 2.f));
    devInputBox_.setFillColor(sf::Color(20, 26, 46));
    devInputBox_.setOutlineColor(sf::Color(140, 160, 210));
    devInputBox_.setOutlineThickness(2.f);

    vol_ = static_cast<float>(SoundManager::instance().volume());
    savedVol_ = vol_;
    updateKnob();
}

bool GlobalHud::onCloseRequested(bool isEscape) {
    // 确认窗已开: Esc 取消, 再点关闭键维持确认窗
    if (exitPopupOpen_) {
        if (isEscape) exitPopupOpen_ = false;
        return true;
    }
    if (!mgr_) return false;
    Room* room = mgr_->room.get();
    // 局内未打完, 或结算界面但本场还有下一局
    const SceneId id = mgr_->currentId();
    const bool inRound = (id == SceneId::Deal || id == SceneId::Arrange || id == SceneId::Battle);
    const bool inMatch = (id == SceneId::Result) && room && !room->isFinished();
    if (!inRound && !inMatch) return false;
    if (!room) return false;
    openExitPopup(inRound);
    return true;
}

void GlobalHud::openExitPopup(bool inRound) {
    popupOpen_ = false;
    devPopupOpen_ = false;
    dragging_ = false;
    devDrag_ = false;
    // 局内说本局, 结算界面说本场
    exitText_.setText(inRound
        ? "本局还未结束，您想要退出吗？\n如果退出，将按逃跑提前结算。"
        : "本场还未结束，您想要退出吗？\n如果退出，将按逃跑提前结算。");
    exitPopupOpen_ = true;
}

void GlobalHud::confirmExit() {
    Room* room = (mgr_ ? mgr_->room.get() : nullptr);
    if (room) {
        // 逃跑罚金 = 倍数 x 本场底注; 直接改筹码, Account::add 内部立即存档
        const int penalty = escapePenaltyFor(room->historyCount, room->config.ante);
        Account::instance().add(-penalty);
    }
    exitPopupOpen_ = false;
    exitConfirmed_ = true;
}

bool GlobalHud::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (mgr_) btnHome_.setVisible(mgr_->homeVisible());

    if (exitPopupOpen_) {
        btnExitOk_.handleEvent(e, win);
        btnExitCancel_.handleEvent(e, win);
        return true;
    }

    bool devPopup = devPopupOpen_;
    if (!devPopup && !popupOpen_) {
        btnMenu_.handleEvent(e, win);
        btnMusic_.handleEvent(e, win);
        btnWrench_.handleEvent(e, win);
        btnHome_.handleEvent(e, win);
        return false;
    }

    if (devPopup) {
        if (devOn_ && devInputFocus_) {
            handleDevText(e);
        }
        if (e.type == sf::Event::MouseButtonPressed &&
            e.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mp = win.mapPixelToCoords(sf::Vector2i(e.mouseButton.x,
                                                                e.mouseButton.y));
            sf::FloatRect ib(devInputBox_.getPosition(), devInputBox_.getSize());
            devInputFocus_ = ib.contains(mp);
            if (devInputFocus_ && devInputStr_.empty()) {
                devInputStr_ = std::to_string(Account::instance().balance());
            }
            if (!devInputFocus_ && devInputStr_.empty()) {
                devInputStr_.clear();
            }
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
    if (e.type == sf::Event::MouseWheelScrolled && showRules_ && rulesScrollMax_ > 0.f) {
        rulesScrollY_ -= e.mouseWheelScroll.delta * RULES_SCROLL_STEP;
        if (rulesScrollY_ < 0.f) rulesScrollY_ = 0.f;
        if (rulesScrollY_ > rulesScrollMax_) rulesScrollY_ = rulesScrollMax_;
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
    // 两段确认超过 5 秒复原
    if (devArm_) {
        devArmTimer_ += dt;
        if (devArmTimer_ >= 5.f) {
            devArm_ = false;
            refreshDevToggle();
        }
    }
}

void GlobalHud::draw(sf::RenderWindow& win) {
    if (mgr_) btnHome_.setVisible(mgr_->homeVisible());
    btnMenu_.draw(win);
    btnMusic_.draw(win);
    btnWrench_.draw(win);
    btnHome_.draw(win);

    if (exitPopupOpen_) {
        win.draw(overlay_);
        win.draw(exitDialog_);
        panel_frame::draw(win, sf::FloatRect(exitDialog_.getPosition(), exitDialog_.getSize()));
        exitText_.draw(win);
        btnExitOk_.draw(win);
        btnExitCancel_.draw(win);
        return;
    }

    if (!devPopupOpen_ && !popupOpen_) return;
    win.draw(overlay_);
    win.draw(panel_);
    panel_frame::draw(win, sf::FloatRect(panel_.getPosition(), panel_.getSize()));
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
        drawRulesText(win);
        btnRulesBack_.draw(win);
        return;
    }
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

void GlobalHud::drawDevPopup(sf::RenderWindow& win) {
    if (!devOn_) {
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
    TextBox ttl("开发者模式", sf::Vector2f(WW / 2.f, PT + 44.f), 28);
    ttl.setColor(sf::Color(255, 220, 130));
    ttl.centerOrigin();
    ttl.draw(win);
    TextBox badge("（已启用开发者模式）", sf::Vector2f(WW / 2.f, PT + 84.f), 20);
    badge.setColor(sf::Color(120, 255, 160));
    badge.centerOrigin();
    badge.draw(win);

    win.draw(devInputBox_);
    TextBox label("直接输入筹码数（0 ~ 100000）", sf::Vector2f(WW / 2.f, DEV_INPUT_Y - DEV_INPUT_H / 2.f - 18.f), 16);
    label.setColor(sf::Color(180, 190, 220));
    label.centerOrigin();
    label.draw(win);
    std::string shown = devInputFocus_ ? devInputStr_ : std::to_string(Account::instance().balance());
    if (devInputFocus_) shown += "|";
    TextBox inputText(shown, sf::Vector2f(WW / 2.f, DEV_INPUT_Y), 24);
    inputText.setColor(sf::Color(255, 255, 255));
    inputText.centerOrigin();
    inputText.draw(win);

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
    devPopupOpen_ = false;
    popupOpen_ = true;
    showRules_ = false;
}

void GlobalHud::openDevPopup() {
    popupOpen_ = false;
    devPopupOpen_ = true;
    devArm_ = false;
    devArmTimer_ = 0.f;
    devInputFocus_ = false;
    devInputStr_.clear();
    refreshDevToggle();
}

void GlobalHud::refreshDevToggle() {
    if (devOn_) {
        btnDevToggle_.setText("关闭开发者模式");
        btnDevToggle_.setTint(sf::Color::White);
    } else if (devArm_) {
        btnDevToggle_.setText("再点一次确认启用");
        btnDevToggle_.setTint(sf::Color(255, 140, 140));
    } else {
        btnDevToggle_.setText("启用开发者模式");
        btnDevToggle_.setTint(sf::Color::White);
    }
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

void GlobalHud::showRulesPage() { showRules_ = true; dragging_ = false; rulesScrollY_ = 0.f; }
void GlobalHud::showMainPage()  { showRules_ = false; }

// 折行后写入 rulesText_, 并算出总高与滚动范围
void GlobalHud::rebuildRulesText() {
    const sf::Font& font = font_util::defaultFont();
    const std::vector<std::string> lines = wrapCjkText(RULES_TEXT, font,
                                                       RULES_FONT_SIZE, RULES_WRAP_W);

    std::string wrapped;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i > 0) wrapped += '\n';
        wrapped += lines[i];
    }

    sf::Text probe;
    probe.setFont(font);
    probe.setCharacterSize(RULES_FONT_SIZE);
    probe.setString(str_util::utf8(wrapped.c_str()));
    const sf::FloatRect b = probe.getLocalBounds();
    // 段高含顶部偏移; 尾部留 8px 余量防止末行贴边
    rulesContentH_ = b.top + b.height + 8.f;

    rulesScrollMax_ = rulesContentH_ - RULES_H;
    if (rulesScrollMax_ < 0.f) rulesScrollMax_ = 0.f;
    rulesScrollY_ = 0.f;
    rulesRtexDirty_ = true;

    rulesText_.setFont(font);
    rulesText_.setCharacterSize(RULES_FONT_SIZE);
    rulesText_.setFillColor(sf::Color(230, 230, 230));
    rulesText_.setString(str_util::utf8(wrapped.c_str()));
    rulesText_.setPosition(sf::Vector2f(RULES_X, RULES_Y));
}

// 放得下直接绘制; 放不下渲染到离屏纹理, 按滚动偏移取窗口显示, 并画滚动条
void GlobalHud::drawRulesText(sf::RenderWindow& win) {
    if (rulesScrollMax_ <= 0.f) {
        win.draw(rulesText_);
        return;
    }
    if (rulesRtexDirty_) {
        const unsigned texW = static_cast<unsigned>(RULES_W) + 2u;
        const unsigned texH = static_cast<unsigned>(rulesContentH_) + 2u;
        if (rulesRtex_.getSize().x != texW || rulesRtex_.getSize().y != texH) {
            rulesRtex_.create(texW, texH);
        }
        rulesRtex_.clear(sf::Color::Transparent);
        rulesText_.setPosition(0.f, 0.f);
        rulesRtex_.draw(rulesText_);
        rulesText_.setPosition(sf::Vector2f(RULES_X, RULES_Y));
        rulesRtex_.display();
        rulesRtexDirty_ = false;
    }
    rulesSprite_.setTexture(rulesRtex_.getTexture());
    rulesSprite_.setTextureRect(sf::IntRect(0, static_cast<int>(rulesScrollY_ + 0.5f),
                                            static_cast<int>(RULES_W),
                                            static_cast<int>(RULES_H)));
    rulesSprite_.setPosition(RULES_X, RULES_Y);
    win.draw(rulesSprite_);

    // 滚动条: 底槽 + 金色滑块
    sf::RectangleShape bar(sf::Vector2f(RULES_BAR_W, RULES_H));
    bar.setPosition(RULES_BAR_X, RULES_Y);
    bar.setFillColor(sf::Color(46, 58, 96));
    win.draw(bar);

    float thumbH = RULES_H * (RULES_H / rulesContentH_);
    if (thumbH < 30.f) thumbH = 30.f;
    if (thumbH > RULES_H) thumbH = RULES_H;
    const float t = rulesScrollY_ / rulesScrollMax_;
    sf::RectangleShape thumb(sf::Vector2f(RULES_BAR_W, thumbH));
    thumb.setPosition(RULES_BAR_X, RULES_Y + t * (RULES_H - thumbH));
    thumb.setFillColor(sf::Color(255, 215, 0));
    win.draw(thumb);
}

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
    applyDevBalance(v);
    if (devInputFocus_) devInputStr_ = std::to_string(v);
}

// 统一写余额: 账号存档 + 在场房间本人筹码, 防两处脱节
void GlobalHud::applyDevBalance(int v) {
    Account::instance().setBalance(v);
    if (mgr_ && mgr_->room && !mgr_->room->isFinished()) {
        mgr_->room->players[0].chips = v;
    }
}

void GlobalHud::applyDevInput() {
    int v = 0;
    if (!devInputStr_.empty()) {
        v = std::atoi(devInputStr_.c_str());
    }
    if (v < 0) v = 0;
    if (v > static_cast<int>(DEV_MAX)) v = static_cast<int>(DEV_MAX);
    applyDevBalance(v);
    devInputStr_ = std::to_string(v);
}

void GlobalHud::handleDevText(const sf::Event& e) {
    if (e.type == sf::Event::TextEntered) {
        sf::Uint32 c = e.text.unicode;
        if (c >= '0' && c <= '9') {
            if (devInputStr_.size() < 6) {
                devInputStr_.push_back(static_cast<char>(c));
            }
            applyDevInput();
        } else if (c == 8) {
            if (!devInputStr_.empty()) {
                devInputStr_.pop_back();
            }
            applyDevInput();
        }
    }
}
