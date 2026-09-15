#pragma once
#include <SFML/Graphics.hpp>

// 功能:横向进度条,随时间流逝填充条缩短,并显示剩余秒数。
// 贴图: countdown_bar_bg 底槽 + 三色填充
// countdown_fill_green / countdown_fill_yellow / countdown_fill_red
// 素材缺失时自动回退为纯色矩形绘制。
// 用法:
// CountdownBar bar; // 10秒
// bar.start;
// 主循环里: bar.update; // dt 为帧间隔秒数
// bar.draw;

class CountdownBar {
public:
    CountdownBar() = default;
    CountdownBar(float maxSeconds, const sf::Vector2f& pos, const sf::Vector2f& size);

    void start();  // 从满时间开始倒计时
    void reset();  // 复位到满时间
    void update(float dt);

    bool isFinished() const { return finished_; }
    float getRemaining() const { return remaining_; }  // 剩余秒数
    float getMax() const { return maxSeconds_; }  // 总时长

    void draw(sf::RenderWindow& win);

private:
    void setRemainingText();  // 更新秒数文字并重新居中

    float maxSeconds_ = 10.f;
    float remaining_ = 10.f;
    float fillWidth_ = 0.f;  // 填充条初始满宽
    bool running_ = false;
    bool finished_ = false;
    sf::Vector2f pos_{0.f, 0.f};
    sf::Vector2f size_{300.f, 30.f};
    sf::RectangleShape bg_;  // 背景槽
    sf::RectangleShape fill_;  // 填充条
    sf::Text label_;  // 剩余秒数文字
};
