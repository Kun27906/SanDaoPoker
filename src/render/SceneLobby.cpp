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

    // 重置账号(右下角; 两段确认: 第一次点击进入确认态, 再点一次执行)
    btnReset_.setText("重置账号");
    btnReset_.setPosition(sf::Vector2f(WW - 200.f, WH - 64.f));
    btnReset_.setSize(sf::Vector2f(180.f, 46.f));
    btnReset_.setCallback([this]() {
        if (!resetArmed_) {
            resetArmed_ = true;
            resetArmTimer_ = 0.f;
            btnReset_.setText("再点一次确认重置");
            btnReset_.setColors(sf::Color(200, 60, 50), sf::Color(240, 90, 70), sf::Color(150, 40, 30));
        } else {
            Account::instance().reset();   // 清空存档并初始化回 500
            resetArmed_ = false;
            btnReset_.setText("重置账号");
            btnReset_.setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
        }
    });
}

void SceneLobby::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    for (auto& b : btnSeats_) b.handleEvent(e, win);
    btnReset_.handleEvent(e, win);
}

void SceneLobby::update(float dt) {
    // 确认态 5 秒未二次点击则复原
    if (resetArmed_) {
        resetArmTimer_ += dt;
        if (resetArmTimer_ >= 5.f) {
            resetArmed_ = false;
            btnReset_.setText("重置账号");
            btnReset_.setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
        }
    }
}

void SceneLobby::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    hint_.draw(win);
    for (auto& b : btnSeats_) b.draw(win);
    btnReset_.draw(win);
    chipBar_.draw(win, Account::instance().balance());
}
