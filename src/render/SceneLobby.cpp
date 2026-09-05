#include "render/SceneLobby.h"
#include "render/AssetManager.h"
#include "render/Account.h"
#include <array>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;
}

SceneLobby::SceneLobby(SceneManager* mgr) : mgr_(mgr) {
    if (const sf::Texture* bg = AssetManager::instance().menuBackground()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

    title_.setText("选择人数");
    title_.setCharacterSize(40);
    title_.setColor(sf::Color(255, 215, 0));
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 60.f));

    hint_.setText("点击人数进入对应房间列表");
    hint_.setCharacterSize(18);
    hint_.setColor(sf::Color(220, 220, 220));
    hint_.centerOrigin();
    hint_.setPosition(sf::Vector2f(WW / 2.f, 110.f));

    const char* names[5] = {"2 人", "3 人", "4 人", "5 人", "6 人"};
    for (int i = 0; i < 5; i++) {
        btnSeats_[i].setText(names[i]);
        btnSeats_[i].setPosition(sf::Vector2f(440.f, 170.f + i * 110.f));
        btnSeats_[i].setSize(sf::Vector2f(400.f, 82.f));
        btnSeats_[i].setCallback([this, i]() {
            mgr_->selectedPlayerCount = i + 2;   // 2..6
            mgr_->changeTo(SceneId::RoomSelect);
        });
    }

    chipBar_.setPosition(sf::Vector2f(WW - 250.f - 20.f, 16.f));
}

void SceneLobby::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    for (auto& b : btnSeats_) b.handleEvent(e, win);
}

void SceneLobby::update(float) {}

void SceneLobby::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    hint_.draw(win);
    for (auto& b : btnSeats_) b.draw(win);
    chipBar_.draw(win, Account::instance().balance());
}
