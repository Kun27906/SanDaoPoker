#include "render/SceneRoomSelect.h"
#include "render/AssetManager.h"
#include "render/SoundManager.h"
#include "render/Account.h"
#include "ui/PanelFrame.h"
#include "core/NameGen.h"
#include "core/Room.h"
#include "core/RuleConfig.h"
#include "ai/AIPlayer.h"

namespace {
constexpr unsigned WW = layout::WINDOW_W;
constexpr unsigned WH = layout::WINDOW_H;

AIPlayer::Difficulty diffOf(int idx) {
    switch (idx) {
        case 0:  return AIPlayer::Difficulty::Random;
        case 2:  return AIPlayer::Difficulty::MonteCarlo;
        default: return AIPlayer::Difficulty::Greedy;
    }
}

int indexOfDiff(AIPlayer::Difficulty d) {
    switch (d) {
        case AIPlayer::Difficulty::Random:     return 0;
        case AIPlayer::Difficulty::MonteCarlo: return 2;
        default:                               return 1;
    }
}

const char* diffDescOf(int idx) {
    switch (idx) {
        case 0:  return "AI 随机分牌, 基本不算牌 -- 适合熟悉规则/练手";
        case 2:  return "AI 蒙特卡洛模拟对手后决策, 精算赢池 -- 硬碰硬";
        default: return "AI 按胜率表贪心分组, 带轻微失误 -- 默认难度";
    }
}
}

SceneRoomSelect::SceneRoomSelect(SceneManager* mgr) : mgr_(mgr) {
    scene_setup::background(bg_, AssetManager::instance().menuBackground(), WW, WH);

    int want = mgr_->selectedPlayerCount;
    if (want < 2) want = 2;
    if (want > 6) want = 6;

    char t[48];
    std::snprintf(t, sizeof(t), "%d 人房间", want);
    title_.setText(t);
    title_.setCharacterSize(36);
    title_.setColor(sf::Color(255, 215, 0));
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 70.f));

    for (int i = 0; i < ROOM_CONFIG_COUNT && roomCount_ < 6; i++) {
        if (ROOM_CONFIGS[i].players == want) {
            roomIndex_[roomCount_++] = i;
        }
    }
    scene_setup::chipBar(chipBar_, WW);
    chipBar_.setImmediate(Account::instance().balance());
    lastBalance_ = Account::instance().balance();

    for (int i = 0; i < roomCount_; i++) {
        roomBtns_[i].setText(ROOM_CONFIGS[roomIndex_[i]].name);
        roomBtns_[i].setPosition(sf::Vector2f(340.f, 160.f + i * 78.f));
        roomBtns_[i].setSize(sf::Vector2f(600.f, 62.f));
        roomBtns_[i].setCallback([this, i]() {
            // 余额不足第一局注金: 弹窗且不允许选中
            const RoomConfig& cfg = ROOM_CONFIGS[roomIndex_[i]];
            const int bal = Account::instance().balance();
            if (bal < cfg.ante) {
                char t[160];
                std::snprintf(t, sizeof(t),
                              "该房间第一局注金为 %d 筹码，\n您的余额只有 %d 筹码，无法进入。",
                              cfg.ante, bal);
                denyText_.setText(t);
                notEnough_ = true;
                return;
            }
            selected_ = i;
            refreshColors();
        });
    }
    refreshColors();

    overlay_.setSize(sf::Vector2f(WW, WH));
    overlay_.setFillColor(sf::Color(0, 0, 0, 160));
    dialog_.setSize(sf::Vector2f(560.f, 230.f));
    dialog_.setPosition(sf::Vector2f((WW - 560.f) / 2.f, (WH - 230.f) / 2.f));
    dialog_.setFillColor(sf::Color(30, 40, 70));
    dialog_.setOutlineColor(sf::Color(255, 170, 60));
    dialog_.setOutlineThickness(3.f);
    denyTitle_.setText("无法进入该房间");
    denyTitle_.setCharacterSize(26);
    denyTitle_.setColor(sf::Color(255, 170, 60));
    denyTitle_.centerOrigin();
    denyTitle_.setPosition(sf::Vector2f(WW / 2.f, (WH - 230.f) / 2.f + 58.f));
    denyText_.setCharacterSize(20);
    denyText_.setColor(sf::Color(235, 235, 235));
    denyText_.centerOrigin();
    denyText_.setPosition(sf::Vector2f(WW / 2.f, (WH - 230.f) / 2.f + 100.f));
    btnDenyOk_.setText("知道了");
    btnDenyOk_.setPosition(sf::Vector2f(WW / 2.f - 80.f, (WH - 230.f) / 2.f + 158.f));
    btnDenyOk_.setSize(sf::Vector2f(160.f, 48.f));
    btnDenyOk_.setCallback([this]() { notEnough_ = false; });

    btnStart_.setText("开始游戏");
    btnStart_.setPosition(sf::Vector2f(495.f, 590.f));
    btnStart_.setSize(sf::Vector2f(290.f, 58.f));
    btnStart_.setCallback([this]() { startGame(); });

    btnDiffOpen_.setPosition(sf::Vector2f(330.f, 590.f));
    btnDiffOpen_.setSize(sf::Vector2f(150.f, 58.f));
    btnDiffOpen_.setCallback([this]() { openDiffPopup(); });

    const float DW = 620.f, DH = 340.f;
    const float DX = (WW - DW) / 2.f, DY = (WH - DH) / 2.f;
    diffDialog_.setSize(sf::Vector2f(DW, DH));
    diffDialog_.setPosition(sf::Vector2f(DX, DY));
    diffDialog_.setFillColor(sf::Color(30, 40, 70));
    diffDialog_.setOutlineColor(sf::Color(150, 130, 255));
    diffDialog_.setOutlineThickness(3.f);

    diffTitle_.setText("选择 AI 难度");
    diffTitle_.setCharacterSize(28);
    diffTitle_.setColor(sf::Color(255, 215, 0));
    diffTitle_.centerOrigin();
    diffTitle_.setPosition(sf::Vector2f(WW / 2.f, DY + 46.f));

    for (int i = 0; i < 3; i++) {
        diffBtns_[i].setText(AIPlayer::difficultyName(diffOf(i)));
        diffBtns_[i].setPosition(sf::Vector2f(WW / 2.f - 272.5f + i * 190.f, DY + 92.f));
        diffBtns_[i].setSize(sf::Vector2f(165.f, 60.f));
        diffBtns_[i].setCallback([this, i]() { applyDifficulty(i); });
    }

    diffDesc_.setCharacterSize(19);
    diffDesc_.setColor(sf::Color(232, 232, 232));
    diffDesc_.centerOrigin();
    diffDesc_.setPosition(sf::Vector2f(WW / 2.f, DY + 196.f));

    btnDiffClose_.setText("完成");
    btnDiffClose_.setPosition(sf::Vector2f(WW / 2.f - 80.f, DY + DH - 62.f));
    btnDiffClose_.setSize(sf::Vector2f(160.f, 46.f));
    btnDiffClose_.setCallback([this]() { diffOpen_ = false; });

    refreshDiffColors();

    scene_setup::selfAvatar(selfAvatar_, WW);
    scene_setup::versionBadge(versionBadge_, WH);

    profile_.bind(&selfAvatar_);
}

void SceneRoomSelect::openDiffPopup() {
    diffOpen_ = true;
    refreshDiffColors();
}

void SceneRoomSelect::applyDifficulty(int idx) {
    AIPlayer::setProfile(diffOf(idx), AIPlayer::Style::Balanced, 0.3f);
    refreshDiffColors();
}

void SceneRoomSelect::refreshDiffColors() {
    const int cur = indexOfDiff(AIPlayer::difficulty());
    for (int i = 0; i < 3; i++) {
        diffBtns_[i].setSelected(i == cur);
    }
    diffDesc_.setText(diffDescOf(cur));
    btnDiffOpen_.setText(std::string("难度: ") + AIPlayer::difficultyName(AIPlayer::difficulty()));
}

void SceneRoomSelect::refreshColors() {
    const int bal = Account::instance().balance();
    for (int i = 0; i < roomCount_; i++) {
        const RoomConfig& cfg = ROOM_CONFIGS[roomIndex_[i]];
        if (bal < cfg.ante) {
            roomBtns_[i].setDisabled(true);
            roomBtns_[i].setSelected(false);
        } else if (i == selected_) {
            roomBtns_[i].setDisabled(false);
            roomBtns_[i].setSelected(true);
        } else {
            roomBtns_[i].setDisabled(false);
            roomBtns_[i].setSelected(false);
        }
    }
}

void SceneRoomSelect::startGame() {
    if (roomCount_ <= 0) return;
    // 入场资格防御
    const RoomConfig& cfg = ROOM_CONFIGS[roomIndex_[selected_]];
    const int bal = Account::instance().balance();
    if (bal < cfg.ante) {
        char t[160];
        std::snprintf(t, sizeof(t),
                      "该房间第一局注金为 %d 筹码，\n您的余额只有 %d 筹码，无法进入。",
                      cfg.ante, bal);
        denyText_.setText(t);
        notEnough_ = true;
        return;
    }
    Account& acct = Account::instance();
    const int entryChips = acct.balance();

    mgr_->room = std::make_unique<Room>();
    if (!mgr_->room->setRoomConfig(roomIndex_[selected_])) return;

    // 真人取存档昵称, AI 随机且同场不重名
    std::string used[MAX_PLAYERS];
    int uc = 0;
    std::string meName = Account::instance().ensureNickname();
    used[uc++] = meName;
    mgr_->room->addPlayer(meName, false);
    for (int i = 1; i < mgr_->room->config.players; i++) {
        std::string nm = makeUniqueNickname(used, uc);
        used[uc++] = nm;
        mgr_->room->addPlayer(nm, true);
    }
    // 真人与 AI 同起点
    for (int i = 0; i < mgr_->room->playerCount; i++) {
        mgr_->room->players[i].chips = entryChips;
    }

    betting_ = true;
    SoundManager::instance().playBet();
    chipBar_.setImmediate(entryChips);
    chipBar_.rollTo(entryChips - cfg.ante, 0.75f);
}

void SceneRoomSelect::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (diffOpen_) {
        for (int i = 0; i < 3; i++) diffBtns_[i].handleEvent(e, win);
        btnDiffClose_.handleEvent(e, win);
        return;
    }
    if (notEnough_) {
        btnDenyOk_.handleEvent(e, win);
        return;
    }
    if (versionBadge_.handleEvent(e, win)) return;
    if (chipBar_.handleEvent(e, win)) return;
    if (profile_.handleEvent(e, win)) return;
    for (int i = 0; i < roomCount_; i++) roomBtns_[i].handleEvent(e, win);
    btnDiffOpen_.handleEvent(e, win);
    btnStart_.handleEvent(e, win);
}

void SceneRoomSelect::onHomePressed() {
    if (diffOpen_) { diffOpen_ = false; return; }
    mgr_->changeTo(SceneId::Lobby);
}

void SceneRoomSelect::update(float dt) {
    chipBar_.update(dt);
    profile_.update(dt);

    // 余额变化则同步房间禁用态与顶部筹码
    const int bal = Account::instance().balance();
    if (bal != lastBalance_) {
        lastBalance_ = bal;
        refreshColors();
        if (!betting_) chipBar_.setImmediate(bal);
    }

    if (betting_ && !chipBar_.isRolling()) {
        mgr_->changeTo(SceneId::Deal);
    }
}

void SceneRoomSelect::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    for (int i = 0; i < roomCount_; i++) roomBtns_[i].draw(win);
    btnDiffOpen_.draw(win);
    btnStart_.draw(win);
    chipBar_.draw(win);
    selfAvatar_.draw(win);
    versionBadge_.draw(win);
    profile_.draw(win);
    if (notEnough_) {
        win.draw(overlay_);
        win.draw(dialog_);
        panel_frame::draw(win, sf::FloatRect(dialog_.getPosition(), dialog_.getSize()));
        denyTitle_.draw(win);
        denyText_.draw(win);
        btnDenyOk_.draw(win);
    }
    if (diffOpen_) {
        win.draw(overlay_);
        win.draw(diffDialog_);
        panel_frame::draw(win, sf::FloatRect(diffDialog_.getPosition(), diffDialog_.getSize()));
        diffTitle_.draw(win);
        for (int i = 0; i < 3; i++) diffBtns_[i].draw(win);
        diffDesc_.draw(win);
        btnDiffClose_.draw(win);
    }
}
