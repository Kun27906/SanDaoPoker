#include "render/SceneRoomSelect.h"
#include "render/AssetManager.h"
#include "render/Account.h"
#include "core/Room.h"
#include "core/RuleConfig.h"
#include "ai/AIPlayer.h"

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;

// ---- 难度选择(成员B): 索引 0/1/2 <-> AIPlayer::Difficulty ----
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
    if (const sf::Texture* bg = AssetManager::instance().menuBackground()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

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

    // 过滤出该人数的房间(保持 ROOM_CONFIGS 原顺序)
    for (int i = 0; i < ROOM_CONFIG_COUNT && roomCount_ < 6; i++) {
        if (ROOM_CONFIGS[i].players == want) {
            roomIndex_[roomCount_++] = i;
        }
    }
    hint_.setText("选择房间(注金为每人每局下注额, 总池均分三道)");
    hint_.setCharacterSize(17);
    hint_.setColor(sf::Color(215, 215, 215));
    hint_.centerOrigin();
    hint_.setPosition(sf::Vector2f(WW / 2.f, 115.f));

    // 房间按钮: 居中一列
    for (int i = 0; i < roomCount_; i++) {
        roomBtns_[i].setText(ROOM_CONFIGS[roomIndex_[i]].name);
        roomBtns_[i].setPosition(sf::Vector2f(340.f, 160.f + i * 78.f));
        roomBtns_[i].setSize(sf::Vector2f(600.f, 62.f));
        roomBtns_[i].setCallback([this, i]() {
            // 入场资格: 余额不足该房间第一局注金(ante)则弹窗提醒,不允许选中
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

    // 入场资格弹窗样式
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
    btnStart_.setPosition(sf::Vector2f(495.f, 590.f));   // 居中(返回大厅改由左上角 home 键)
    btnStart_.setSize(sf::Vector2f(290.f, 58.f));
    btnStart_.setCallback([this]() { startGame(); });

    // ---- 难度选择(成员B): 入口按钮 + 居中弹窗 ----
    btnDiffOpen_.setPosition(sf::Vector2f(330.f, 590.f));
    btnDiffOpen_.setSize(sf::Vector2f(150.f, 58.f));
    btnDiffOpen_.setColors(sf::Color(96, 84, 160), sf::Color(126, 112, 200), sf::Color(70, 60, 122));
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

    chipBar_.setPosition(sf::Vector2f(WW - 250.f - 20.f, 16.f));
}

// ---- 难度选择(成员B) ----
void SceneRoomSelect::openDiffPopup() {
    diffOpen_ = true;
    refreshDiffColors();
}

void SceneRoomSelect::applyDifficulty(int idx) {
    // 三档难度立刻写入 AI 模块(均衡风格 + 0.3 失误率, 保留人味)
    AIPlayer::setProfile(diffOf(idx), AIPlayer::Style::Balanced, 0.3f);
    refreshDiffColors();
}

void SceneRoomSelect::refreshDiffColors() {
    const int cur = indexOfDiff(AIPlayer::difficulty());
    for (int i = 0; i < 3; i++) {
        if (i == cur) {
            diffBtns_[i].setColors(sf::Color(46, 160, 80), sf::Color(70, 190, 100), sf::Color(30, 120, 55));
        } else {
            diffBtns_[i].setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
        }
    }
    diffDesc_.setText(diffDescOf(cur));
    btnDiffOpen_.setText(std::string("难度: ") + AIPlayer::difficultyName(AIPlayer::difficulty()));
}

void SceneRoomSelect::refreshColors() {
    const int bal = Account::instance().balance();
    for (int i = 0; i < roomCount_; i++) {
        const RoomConfig& cfg = ROOM_CONFIGS[roomIndex_[i]];
        if (bal < cfg.ante) {
            // 余额不足该房间第一局注金: 灰色(不可进入,点击仍弹提示)
            roomBtns_[i].setColors(sf::Color(88, 92, 104), sf::Color(104, 108, 122), sf::Color(66, 70, 82));
        } else if (i == selected_) {
            roomBtns_[i].setColors(sf::Color(46, 160, 80), sf::Color(70, 190, 100), sf::Color(30, 120, 55));
        } else {
            roomBtns_[i].setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
        }
    }
}

void SceneRoomSelect::startGame() {
    if (roomCount_ <= 0) return;
    // 入场资格防御: 余额不足第一局注金则弹窗(正常点击房间时已拦截,双保险)
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
    // 入场筹码 = 账号余额(进入本界面必经大厅, 大厅已保证余额>=100 且破产已弹窗补充)
    Account& acct = Account::instance();
    const int entryChips = acct.balance();

    mgr_->room = std::make_unique<Room>();
    if (!mgr_->room->setRoomConfig(roomIndex_[selected_])) return;

    mgr_->room->addPlayer("你", false);
    for (int i = 1; i < mgr_->room->config.players; i++) {
        char name[16];
        std::snprintf(name, sizeof(name), "AI-%d", i);
        mgr_->room->addPlayer(name, true);
    }
    // 入场筹码: 真人与 AI 同起点 = 账号余额
    for (int i = 0; i < mgr_->room->playerCount; i++) {
        mgr_->room->players[i].chips = entryChips;
    }
    mgr_->changeTo(SceneId::Arrange);
}

void SceneRoomSelect::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (diffOpen_) {                   // 难度选择弹窗:只响应弹窗内按钮
        for (int i = 0; i < 3; i++) diffBtns_[i].handleEvent(e, win);
        btnDiffClose_.handleEvent(e, win);
        return;
    }
    if (notEnough_) {                  // 入场资格弹窗:只响应确定
        btnDenyOk_.handleEvent(e, win);
        return;
    }
    for (int i = 0; i < roomCount_; i++) roomBtns_[i].handleEvent(e, win);
    btnDiffOpen_.handleEvent(e, win);
    btnStart_.handleEvent(e, win);
}

void SceneRoomSelect::onHomePressed() {
    if (diffOpen_) { diffOpen_ = false; return; }   // 难度弹窗打开时先关弹窗
    mgr_->changeTo(SceneId::Lobby);   // home 键返回大厅
}

void SceneRoomSelect::update(float) {}

void SceneRoomSelect::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    hint_.draw(win);
    for (int i = 0; i < roomCount_; i++) roomBtns_[i].draw(win);
    btnDiffOpen_.draw(win);
    btnStart_.draw(win);
    chipBar_.draw(win, Account::instance().balance());
    if (notEnough_) {                  // 入场资格弹窗
        win.draw(overlay_);
        win.draw(dialog_);
        denyTitle_.draw(win);
        denyText_.draw(win);
        btnDenyOk_.draw(win);
    }
    if (diffOpen_) {                   // 难度选择弹窗
        win.draw(overlay_);
        win.draw(diffDialog_);
        diffTitle_.draw(win);
        for (int i = 0; i < 3; i++) diffBtns_[i].draw(win);
        diffDesc_.draw(win);
        btnDiffClose_.draw(win);
    }
}
