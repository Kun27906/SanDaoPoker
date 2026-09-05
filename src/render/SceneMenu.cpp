#include "render/SceneMenu.h"
#include "render/AssetManager.h"
#include "core/Room.h"
#include "core/RuleConfig.h"
#include <cstdio>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;

// 房间按钮文字:直接用房间名(已含人数/注金/局数,如 "4人·注500·6局")
std::string roomLabel(int i) {
    return std::string(ROOM_CONFIGS[i].name);
}
}

SceneMenu::SceneMenu(SceneManager* mgr) : mgr_(mgr) {
    // 背景(主菜单专用背景,失败自动回退到桌面背景)
    if (const sf::Texture* bg = AssetManager::instance().menuBackground()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

    title_.setText("炸金花三道 - 选择房间");
    title_.setCharacterSize(44);
    title_.setColor(sf::Color::White);
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 50.f));

    subtitle_.setText("点击房间选中(默认4人·注500·6局), 然后点[开始游戏]");
    subtitle_.setCharacterSize(18);
    subtitle_.setColor(sf::Color(210, 210, 210));
    subtitle_.centerOrigin();
    subtitle_.setPosition(sf::Vector2f(WW / 2.f, 92.f));

    // 22 个房间按钮: 两列 x 11 行
    for (int i = 0; i < 22; i++) {
        int col = i / 11;              // 0=左列 1=右列
        int row = i % 11;              // 0..10
        float x = 110.f + col * 560.f;
        float y = 128.f + row * 48.f;
        roomBtns_[i].setText(roomLabel(i));
        roomBtns_[i].setPosition(sf::Vector2f(x, y));
        roomBtns_[i].setSize(sf::Vector2f(520.f, 40.f));
        roomBtns_[i].setCharacterSize(20);
        roomBtns_[i].setCallback([this, i]() {
            selected_ = i;
            refreshRoomButtonColors();
        });
    }
    refreshRoomButtonColors();

    // 开始游戏按钮
    btnStart_.setText("开始游戏");
    btnStart_.setPosition(sf::Vector2f(440.f, 690.f));
    btnStart_.setSize(sf::Vector2f(400.f, 60.f));
    btnStart_.setCallback([this]() { startGame(); });
}

void SceneMenu::refreshRoomButtonColors() {
    for (int i = 0; i < 22; i++) {
        if (i == selected_) {
            // 选中:绿色系
            roomBtns_[i].setColors(sf::Color(46, 160, 80), sf::Color(70, 190, 100), sf::Color(30, 120, 55));
        } else {
            // 未选中:默认蓝色系
            roomBtns_[i].setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
        }
    }
}

void SceneMenu::startGame() {
    // 创建房间并加入玩家(1 真人 + 其余 AI)
    mgr_->room = std::make_unique<Room>();
    if (!mgr_->room->setRoomConfig(selected_)) {
        subtitle_.setText("房间配置无效, 请重试");
        return;
    }
    mgr_->room->addPlayer("你", false);
    for (int i = 1; i < mgr_->room->config.players; i++) {
        char name[16];
        std::snprintf(name, sizeof(name), "AI-%d", i);
        mgr_->room->addPlayer(name, true);
    }
    // 进入组牌界面
    mgr_->changeTo(SceneId::Arrange);
}

void SceneMenu::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    for (auto& b : roomBtns_) b.handleEvent(e, win);
    btnStart_.handleEvent(e, win);
}

void SceneMenu::update(float) {}

void SceneMenu::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    subtitle_.draw(win);
    for (auto& b : roomBtns_) b.draw(win);
    btnStart_.draw(win);
}
