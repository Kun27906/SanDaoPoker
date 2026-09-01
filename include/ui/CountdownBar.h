#pragma once
#include <SFML/Graphics.hpp>

// ====== CountdownBar 倒计时条控件 ======
// 功能:横向进度条,随时间流逝填充条缩短,并显示剩余秒数。
//       最后 1/3 时间变红(警示),用于组牌界面的交牌限时。
// 用法:
//   CountdownBar bar(10.f, {100,100}, {300,30});  // 10秒
//   bar.start();
//   主循环里: bar.update(dt);   // dt 为帧间隔秒数
//   bar.draw(window);

class CountdownBar {
public:
    CountdownBar() = default;
    CountdownBar(float maxSeconds, const sf::Vector2f& pos, const sf::Vector2f& size);

    void start();   // 从满时间开始倒计时
    void reset();   // 复位到满时间(不自动开始)
    void update(float dt);

    bool isRunning() const { return running_; }
    bool isFinished() const { return finished_; }
    float getRemaining() const { return remaining_; }
    float getMax() const { return maxSeconds_; }

    void draw(sf::RenderWindow& win);

private:
    void setRemainingText();  // 更新秒数文字并重新居中

    float maxSeconds_ = 10.f;
    float remaining_ = 10.f;
    bool running_ = false;
    bool finished_ = false;
    sf::RectangleShape bg_;    // 背景槽
    sf::RectangleShape fill_;  // 填充条
    sf::Text label_;           // 剩余秒数文字
};
