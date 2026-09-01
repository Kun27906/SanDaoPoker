#include "render/SceneResult.h"
#include "render/AssetManager.h"

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;
}

SceneResult::SceneResult(SceneManager* mgr) : mgr_(mgr) {
    if (const sf::Texture* bg = AssetManager::instance().background()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

    title_.setText("结算界面(阶段3骨架)");
    title_.setCharacterSize(44);
    title_.setColor(sf::Color::White);
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 200.f));

    hint_.setText("阶段7将实现: 筹码变化 + 总账排名");
    hint_.setCharacterSize(20);
    hint_.setColor(sf::Color(210, 210, 210));
    hint_.centerOrigin();
    hint_.setPosition(sf::Vector2f(WW / 2.f, 260.f));

    btnBack_.setText("返回主菜单");
    btnBack_.setPosition(sf::Vector2f(440.f, 420.f));
    btnBack_.setSize(sf::Vector2f(400.f, 56.f));
    btnBack_.setCallback([this]() { mgr_->changeTo(SceneId::Menu); });
}

void SceneResult::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    btnBack_.handleEvent(e, win);
}

void SceneResult::update(float) {}

void SceneResult::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    hint_.draw(win);
    btnBack_.draw(win);
}
