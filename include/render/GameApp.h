#pragma once
#include <SFML/Graphics.hpp>
#include "render/SceneManager.h"

// ====== GameApp 游戏主控(阶段2/3) ======
// 阶段0/1:SFML 窗口 + 主循环 + 控件测试台(已退役)
// 阶段2  :素材加载(AssetManager)+牌精灵(CardSprite)   [本阶段]
// 阶段3  :场景状态机(SceneManager + 四场景骨架)       [本阶段]
// 后续   :阶段4选房 -> 5组牌 -> 6比牌 -> 7结算

class GameApp {
public:
    GameApp();
    void run();

private:
    sf::RenderWindow window_;
    sf::Clock clock_;
    SceneManager sceneManager_;
};
