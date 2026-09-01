#include "ui/CountdownBar.h"
#include "ui/FontUtil.h"
#include <cmath>
#include <cstdio>

CountdownBar::CountdownBar(float maxSeconds, const sf::Vector2f& pos, const sf::Vector2f& size) {
    maxSeconds_ = maxSeconds > 0.f ? maxSeconds : 1.f;
    remaining_ = maxSeconds_;

    // 背景槽:深灰底 + 边框
    bg_.setSize(size);
    bg_.setPosition(pos);
    bg_.setFillColor(sf::Color(60, 60, 60));
    bg_.setOutlineColor(sf::Color(180, 180, 180));
    bg_.setOutlineThickness(2.f);

    // 填充条:初始为满宽,绿色
    fillWidth_ = size.x - 4.f;
    fill_.setSize(sf::Vector2f(fillWidth_, size.y - 4.f));
    fill_.setPosition(pos.x + 2.f, pos.y + 2.f);
    fill_.setFillColor(sf::Color(60, 180, 80));

    // 秒数文字:显示在条中央
    label_.setFont(font_util::defaultFont());
    label_.setCharacterSize(18);
    label_.setFillColor(sf::Color::White);
    sf::FloatRect lb = label_.getLocalBounds();
    label_.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
    label_.setPosition(pos.x + size.x / 2.f, pos.y + size.y / 2.f);
    setRemainingText();
}

void CountdownBar::start() {
    remaining_ = maxSeconds_;
    running_ = true;
    finished_ = false;
    setRemainingText();
}

void CountdownBar::reset() {
    remaining_ = maxSeconds_;
    running_ = false;
    finished_ = false;
    setRemainingText();
}

void CountdownBar::update(float dt) {
    if (!running_) return;
    remaining_ -= dt;
    if (remaining_ <= 0.f) {
        remaining_ = 0.f;
        running_ = false;
        finished_ = true;
    }
    setRemainingText();
}

void CountdownBar::draw(sf::RenderWindow& win) {
    win.draw(bg_);

    // 填充条宽度按剩余比例(基于初始满宽,避免逐帧乘当前宽度导致的指数衰减)
    float ratio = maxSeconds_ > 0.f ? (remaining_ / maxSeconds_) : 0.f;
    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;
    fill_.setSize(sf::Vector2f(fillWidth_ * ratio, fill_.getSize().y));

    // 颜色:前 2/3 绿色,后 1/3 红色(警示)
    fill_.setFillColor(ratio > 0.33f ? sf::Color(60, 180, 80) : sf::Color(220, 60, 50));

    win.draw(fill_);
    win.draw(label_);
}

void CountdownBar::setRemainingText() {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%.1f", remaining_);
    label_.setString(buf);
    // 重新居中
    sf::FloatRect lb = label_.getLocalBounds();
    label_.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
}
