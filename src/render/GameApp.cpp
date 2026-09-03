#include "render/GameApp.h"
#include "render/AssetManager.h"
#include "render/SoundManager.h"
#include "ui/FontUtil.h"

namespace {
constexpr unsigned WINDOW_W = 1280;
constexpr unsigned WINDOW_H = 800;

// 窗口标题(中文需显式 UTF-8 转换)
sf::String windowTitle() {
    return str_util::utf8("SanDaoPoker - 炸金花三道");
}
}

GameApp::GameApp()
    : window_(sf::VideoMode(WINDOW_W, WINDOW_H), windowTitle()),
      sceneManager_(window_) {
    // 加载全部素材(牌图/牌背/背景/按钮图);失败不致命,相关位置显示占位
    AssetManager::instance().loadAll();
    // 加载全部音效(发牌/翻牌/筹码/胜负/按钮点击;失败静默)
    SoundManager::instance().loadAll();
    window_.setFramerateLimit(60);
}

void GameApp::run() {
    while (window_.isOpen()) {
        float dt = clock_.restart().asSeconds();
        sf::Event e;
        while (window_.pollEvent(e)) {
            if (e.type == sf::Event::Closed) {
                window_.close();
            } else if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
                window_.close();
            }
            sceneManager_.handleEvent(e, window_);
        }
        sceneManager_.update(dt);
        window_.clear(sf::Color(20, 24, 30));
        sceneManager_.draw(window_);
        window_.display();
    }
}
