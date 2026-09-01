#pragma once
#include <SFML/Graphics.hpp>
#include "ui/Button.h"
#include "ui/TextBox.h"
#include "ui/CountdownBar.h"

// ====== GameApp 游戏主控(阶段 0/1) ======
// 阶段 0:SFML 窗口 + 主循环(事件/更新/渲染) + 可关闭
// 阶段 1:通用控件测试台(Button / TextBox / CountdownBar)
//   - Start Countdown  : 启动 10 秒倒计时条
//   - Reset            : 复位倒计时
//   - 倒计时走完        : 状态文字提示(模拟组牌超时交牌)
// 后续阶段:在此基础上升级为 SceneManager 四场景状态机。

class GameApp {
public:
    GameApp();
    void run();

private:
    void handleEvents();
    void update(float dt);
    void render();

    sf::RenderWindow window_;
    sf::Clock clock_;

    // ---- 阶段 1 测试台控件 ----
    TextBox titleText_;      // 顶部标题
    TextBox subtitleText_;   // 副标题
    TextBox statusText_;     // 状态文字
    Button btnStart_;        // 开始倒计时
    Button btnReset_;        // 复位
    CountdownBar countdown_; // 倒计时条(10秒)
};
