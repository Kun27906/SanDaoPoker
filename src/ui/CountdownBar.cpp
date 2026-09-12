#include "ui/CountdownBar.h"
#include "ui/FontUtil.h"
#include "render/AssetManager.h"
#include <cmath>
#include <cstdio>

CountdownBar::CountdownBar(float maxSeconds, const sf::Vector2f& pos, const sf::Vector2f& size) {
    maxSeconds_ = maxSeconds > 0.f ? maxSeconds : 1.f;
    remaining_ = maxSeconds_;
    pos_ = pos;
    size_ = size;

    // 背景槽:深灰底 + 边框(贴图缺失时回退用)
    bg_.setSize(size);
    bg_.setPosition(pos);
    bg_.setFillColor(sf::Color(60, 60, 60));
    bg_.setOutlineColor(sf::Color(180, 180, 180));
    bg_.setOutlineThickness(2.f);

    // 填充条:初始为满宽,绿色(贴图缺失时回退用)
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
    // 剩余比例(基于初始满宽,避免逐帧乘当前宽度导致的指数衰减)
    float ratio = maxSeconds_ > 0.f ? (remaining_ / maxSeconds_) : 0.f;
    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;

    // ---- 底槽: 贴图优先, 缺失则纯色矩形 ----
    const sf::Texture* bgTex = AssetManager::instance().tableTexture("countdown_bar_bg");
    if (bgTex) {
        sf::Sprite s(*bgTex);
        s.setScale(size_.x / bgTex->getSize().x, size_.y / bgTex->getSize().y);
        s.setPosition(pos_);
        win.draw(s);
    } else {
        win.draw(bg_);
    }

    // ---- 填充: 三色贴图(绿>50%, 黄15%~50%, 红<15%), 按剩余比例裁切 ----
    const char* fillName = ratio > 0.50f ? "countdown_fill_green"
                         : ratio > 0.15f ? "countdown_fill_yellow"
                                         : "countdown_fill_red";
    const sf::Texture* fillTex = AssetManager::instance().tableTexture(fillName);
    if (fillTex) {
        float tw = static_cast<float>(fillTex->getSize().x);
        float th = static_cast<float>(fillTex->getSize().y);
        int visW = static_cast<int>(tw * ratio);
        if (visW > 0) {
            sf::Sprite f(*fillTex);
            f.setTextureRect(sf::IntRect(0, 0, visW, static_cast<int>(th)));
            f.setScale(fillWidth_ / tw, (size_.y - 4.f) / th);   // x 缩放为常数(比例只在裁剪上体现)
            f.setPosition(pos_.x + 2.f, pos_.y + 2.f);
            win.draw(f);
        }
    } else {
        fill_.setSize(sf::Vector2f(fillWidth_ * ratio, fill_.getSize().y));
        fill_.setFillColor(ratio > 0.50f ? sf::Color(60, 180, 80)
                         : ratio > 0.15f ? sf::Color(230, 190, 40)
                                         : sf::Color(220, 60, 50));
        win.draw(fill_);
    }

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
