#include "render/SceneMenu.h"
#include "render/AssetManager.h"

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;
}

SceneMenu::SceneMenu(SceneManager* mgr) : mgr_(mgr) {
    // 背景(2000x1200 缩放到窗口)
    if (const sf::Texture* bg = AssetManager::instance().background()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

    // 标题
    title_.setText("炸金花三道");
    title_.setCharacterSize(72);
    title_.setColor(sf::Color::White);
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 180.f));

    subtitle_.setText("主菜单(阶段3骨架: 场景切换演示)");
    subtitle_.setCharacterSize(22);
    subtitle_.setColor(sf::Color(220, 220, 220));
    subtitle_.centerOrigin();
    subtitle_.setPosition(sf::Vector2f(WW / 2.f, 240.f));

    // 入口按钮
    btnArrange_.setText("进入组牌界面");
    btnArrange_.setPosition(sf::Vector2f(440.f, 360.f));
    btnArrange_.setSize(sf::Vector2f(400.f, 60.f));
    btnArrange_.setCallback([this]() { mgr_->changeTo(SceneId::Arrange); });

    btnBattle_.setText("进入比牌界面");
    btnBattle_.setPosition(sf::Vector2f(440.f, 450.f));
    btnBattle_.setSize(sf::Vector2f(400.f, 60.f));
    btnBattle_.setCallback([this]() { mgr_->changeTo(SceneId::Battle); });

    btnResult_.setText("进入结算界面");
    btnResult_.setPosition(sf::Vector2f(440.f, 540.f));
    btnResult_.setSize(sf::Vector2f(400.f, 60.f));
    btnResult_.setCallback([this]() { mgr_->changeTo(SceneId::Result); });
}

void SceneMenu::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    btnArrange_.handleEvent(e, win);
    btnBattle_.handleEvent(e, win);
    btnResult_.handleEvent(e, win);
}

void SceneMenu::update(float) {}

void SceneMenu::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    subtitle_.draw(win);
    btnArrange_.draw(win);
    btnBattle_.draw(win);
    btnResult_.draw(win);
}
