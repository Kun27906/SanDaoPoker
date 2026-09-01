#include "render/GameApp.h"

namespace {
constexpr unsigned WINDOW_W = 1024;
constexpr unsigned WINDOW_H = 768;
}

GameApp::GameApp()
    : window_(sf::VideoMode(WINDOW_W, WINDOW_H), "SanDaoPoker v0.1 - UI Test Bench") {
    // ---- 标题 ----
    titleText_.setText("SanDaoPoker v0.1");
    titleText_.setCharacterSize(40);
    titleText_.setColor(sf::Color::White);
    titleText_.centerOrigin();
    titleText_.setPosition(sf::Vector2f(WINDOW_W / 2.f, 70.f));

    // ---- 副标题 ----
    subtitleText_.setText("Stage 0/1: Window + UI Controls Test Bench");
    subtitleText_.setCharacterSize(18);
    subtitleText_.setColor(sf::Color(180, 180, 180));
    subtitleText_.centerOrigin();
    subtitleText_.setPosition(sf::Vector2f(WINDOW_W / 2.f, 115.f));

    // ---- 按钮:开始倒计时 ----
    btnStart_.setText("Start Countdown");
    btnStart_.setPosition(sf::Vector2f(312.f, 320.f));
    btnStart_.setSize(sf::Vector2f(400.f, 56.f));
    btnStart_.setCallback([this]() {
        countdown_.start();
        statusText_.setText("Counting down... (simulating arrange timeout)");
        statusText_.setColor(sf::Color(120, 200, 120));
    });

    // ---- 按钮:复位 ----
    btnReset_.setText("Reset");
    btnReset_.setPosition(sf::Vector2f(312.f, 400.f));
    btnReset_.setSize(sf::Vector2f(400.f, 56.f));
    btnReset_.setCallback([this]() {
        countdown_.reset();
        statusText_.setText("Ready. Press Start Countdown.");
        statusText_.setColor(sf::Color(200, 200, 200));
    });

    // ---- 倒计时条(10秒) ----
    countdown_ = CountdownBar(10.f, sf::Vector2f(312.f, 500.f), sf::Vector2f(400.f, 36.f));

    // ---- 状态文字 ----
    statusText_.setText("Ready. Press Start Countdown.");
    statusText_.setCharacterSize(20);
    statusText_.setColor(sf::Color(200, 200, 200));
    statusText_.centerOrigin();
    statusText_.setPosition(sf::Vector2f(WINDOW_W / 2.f, 590.f));
}

void GameApp::run() {
    while (window_.isOpen()) {
        float dt = clock_.restart().asSeconds();
        handleEvents();
        update(dt);
        render();
    }
}

void GameApp::handleEvents() {
    sf::Event e;
    while (window_.pollEvent(e)) {
        if (e.type == sf::Event::Closed) {
            window_.close();
        } else if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
            window_.close();
        }
        btnStart_.handleEvent(e, window_);
        btnReset_.handleEvent(e, window_);
    }
}

void GameApp::update(float dt) {
    countdown_.update(dt);

    // 倒计时刚结束:提示一次(模拟组牌超时自动交牌)
    static bool wasFinished = false;
    if (countdown_.isFinished() && !wasFinished) {
        statusText_.setText("Time's up! (auto-submit would happen here)");
        statusText_.setColor(sf::Color(220, 90, 70));
    }
    wasFinished = countdown_.isFinished();
}

void GameApp::render() {
    window_.clear(sf::Color(30, 34, 40));  // 深色背景

    titleText_.draw(window_);
    subtitleText_.draw(window_);
    btnStart_.draw(window_);
    btnReset_.draw(window_);
    countdown_.draw(window_);
    statusText_.draw(window_);

    window_.display();
}
