#pragma once
#include <SFML/Graphics.hpp>

// 横向倒计时条, 素材缺失时回退纯色矩形
class CountdownBar {
public:
    CountdownBar() = default;
    CountdownBar(float maxSeconds, const sf::Vector2f& pos, const sf::Vector2f& size);

    void start();
    void reset();
    void update(float dt);

    bool isFinished() const { return finished_; }
    float getRemaining() const { return remaining_; }
    float getMax() const { return maxSeconds_; }

    void draw(sf::RenderWindow& win);

private:
    void setRemainingText();

    float maxSeconds_ = 10.f;
    float remaining_ = 10.f;
    float fillWidth_ = 0.f;
    bool running_ = false;
    bool finished_ = false;
    sf::Vector2f pos_{0.f, 0.f};
    sf::Vector2f size_{300.f, 30.f};
    sf::RectangleShape bg_;
    sf::RectangleShape fill_;
    sf::Text label_;
};
