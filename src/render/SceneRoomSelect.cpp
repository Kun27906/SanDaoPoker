#include "render/SceneRoomSelect.h"
#include "render/AssetManager.h"
#include "render/Account.h"
#include "core/Room.h"
#include "core/RuleConfig.h"

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;
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
            selected_ = i;
            refreshColors();
        });
    }
    refreshColors();

    btnStart_.setText("开始游戏");
    btnStart_.setPosition(sf::Vector2f(340.f, 590.f));
    btnStart_.setSize(sf::Vector2f(290.f, 58.f));
    btnStart_.setCallback([this]() { startGame(); });

    btnBack_.setText("返回大厅");
    btnBack_.setPosition(sf::Vector2f(650.f, 590.f));
    btnBack_.setSize(sf::Vector2f(290.f, 58.f));
    btnBack_.setCallback([this]() { mgr_->changeTo(SceneId::Lobby); });

    chipBar_.setPosition(sf::Vector2f(WW - 250.f - 20.f, 16.f));
}

void SceneRoomSelect::refreshColors() {
    for (int i = 0; i < roomCount_; i++) {
        if (i == selected_) {
            roomBtns_[i].setColors(sf::Color(46, 160, 80), sf::Color(70, 190, 100), sf::Color(30, 120, 55));
        } else {
            roomBtns_[i].setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
        }
    }
}

void SceneRoomSelect::startGame() {
    if (roomCount_ <= 0) return;
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
    for (int i = 0; i < roomCount_; i++) roomBtns_[i].handleEvent(e, win);
    btnStart_.handleEvent(e, win);
    btnBack_.handleEvent(e, win);
}

void SceneRoomSelect::update(float) {}

void SceneRoomSelect::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    hint_.draw(win);
    for (int i = 0; i < roomCount_; i++) roomBtns_[i].draw(win);
    btnStart_.draw(win);
    btnBack_.draw(win);
    chipBar_.draw(win, Account::instance().balance());
}
