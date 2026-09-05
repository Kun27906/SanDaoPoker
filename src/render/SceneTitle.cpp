#include "render/SceneTitle.h"
#include "render/AssetManager.h"

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;
}

SceneTitle::SceneTitle(SceneManager* mgr) : mgr_(mgr) {
    if (const sf::Texture* bg = AssetManager::instance().menuBackground()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

    title_.setText("炸金花三道");
    title_.setCharacterSize(72);
    title_.setColor(sf::Color(255, 215, 0));
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 230.f));

    subtitle_.setText("经典比牌 · 2~6人 · 本地账号");
    subtitle_.setCharacterSize(22);
    subtitle_.setColor(sf::Color(230, 230, 230));
    subtitle_.centerOrigin();
    subtitle_.setPosition(sf::Vector2f(WW / 2.f, 310.f));

    version_.setText("v0.2");
    version_.setCharacterSize(16);
    version_.setColor(sf::Color(180, 180, 180));
    version_.setPosition(sf::Vector2f(WW - 60.f, WH - 34.f));

    btnStart_.setText("开始游戏");
    btnStart_.setPosition(sf::Vector2f(440.f, 420.f));
    btnStart_.setSize(sf::Vector2f(400.f, 70.f));
    btnStart_.setCallback([this]() { mgr_->changeTo(SceneId::Lobby); });
}

void SceneTitle::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    btnStart_.handleEvent(e, win);
}

void SceneTitle::update(float) {}

void SceneTitle::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    subtitle_.draw(win);
    version_.draw(win);
    btnStart_.draw(win);
}
