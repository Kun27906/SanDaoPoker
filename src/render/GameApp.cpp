#include "render/GameApp.h"
#include "render/Layout.h"
#include "render/AssetManager.h"
#include "render/SoundManager.h"
#include "ai/AIPlayer.h"
#include "ui/FontUtil.h"
#include <cstdio>

namespace {
constexpr unsigned WINDOW_W = layout::WINDOW_W;
constexpr unsigned WINDOW_H = layout::WINDOW_H;

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
    // 加载 AI 胜率表(组牌决策: 24804 种三张牌组合 vs 随机对手胜率)
    // 失败不致命: 打印错误, AI 评分退化为 0(启动后应保证该素材存在)
    if (!AIPlayer::loadWinRateTable("assets/ai/winrate.bin")) {
        std::fprintf(stderr, "[AI] 胜率表加载失败: assets/ai/winrate.bin\n");
    }
    // 启动即循环播放主菜单 BGM
    SoundManager::instance().playBgmMenu();
    window_.setFramerateLimit(60);
}

void GameApp::run() {
    while (window_.isOpen()) {
        float dt = clock_.restart().asSeconds();
        sf::Event e;
        while (window_.pollEvent(e)) {
            // 关闭窗口(标题栏 X / Esc): 局内"一局未结束"(发牌/组牌/比牌)时先弹确认窗,
            // 点[确定]才按逃跑提前结算并退出; 其余场景保持原行为直接退出。
            if (e.type == sf::Event::Closed) {
                if (!sceneManager_.onCloseRequested(false)) window_.close();
            } else if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
                if (!sceneManager_.onCloseRequested(true)) window_.close();
            }
            sceneManager_.handleEvent(e, window_);
        }
        sceneManager_.update(dt);
        if (sceneManager_.shouldClose()) window_.close();   // 确认窗点[确定] -> 退出
        window_.clear(sf::Color(20, 24, 30));
        sceneManager_.draw(window_);
        window_.display();
    }
}
