#pragma once
#include <SFML/Graphics.hpp>

// 右上角筹码框, 数字可滚动跳动
class ChipBar {
public:
    ChipBar();
    void setPosition(const sf::Vector2f& pos);

    void setImmediate(int v);
    void rollTo(int v, float seconds = 0.6f);
    void update(float dt);
    bool isRolling() const { return rolling_; }

    void draw(sf::RenderWindow& win);

    // 点击筹码图标播 chip 音效, 返回 true 表示已消费
    bool handleEvent(const sf::Event& e, const sf::RenderWindow& win);

private:
    sf::Vector2f pos_;
    sf::Text text_;
    sf::Text label_;
    int shown_ = 0;
    int from_ = 0;
    int target_ = 0;
    float t_ = 0.f;
    float dur_ = 0.6f;
    bool rolling_ = false;
    bool inited_ = false;
    int cachedShown_ = -1;
};
