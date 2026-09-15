#include "render/SceneTitle.h"
#include "render/AssetManager.h"

namespace {
constexpr unsigned WW = layout::WINDOW_W;
constexpr unsigned WH = layout::WINDOW_H;
}

SceneTitle::SceneTitle(SceneManager* mgr): mgr_(mgr) {
    scene_setup::background(bg_, AssetManager::instance().menuBackground(), WW, WH);

    title_.setText("炸金花三道");
    title_.setCharacterSize(72);
    title_.setColor(sf::Color(255, 215, 0));
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 230.f));

    subtitle_.setText("经典比牌 · 2~6人 · 本地账号");
    subtitle_.setCharacterSize(22);
    subtitle_.setColor(sf::Color(255, 240, 170));  // 亮浅金
    subtitle_.centerOrigin();
    subtitle_.setPosition(sf::Vector2f(WW / 2.f, 310.f));

    scene_setup::versionBadge(versionBadge_, WH);

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
    versionBadge_.draw(win);
    btnStart_.draw(win);
}
