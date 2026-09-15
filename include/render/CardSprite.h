#pragma once
#include <SFML/Graphics.hpp>
#include "core/Card.h"

// 把 A 成员的核心牌对象显示为界面上的牌图。
// - 正面:按 花色/点数 从 AssetManager 取贴图
// - 牌背:setFaceUp 显示背面
// - 大小王:贴图未就绪时自动用牌背兜底
// 用法:
// CardSprite cs;
// cs.setCard;
// cs.setPosition;
// cs.setScale; // 200x280 -> 100x140
// cs.draw;

class CardSprite {
public:
    CardSprite() = default;

  // 设置要显示的牌
    void setCard(const Card& card);
  // 正面/牌背
    void setFaceUp(bool up) { faceUp_ = up; }

    void setPosition(const sf::Vector2f& p);
    void setScale(float s);  // 整体缩放
    sf::Vector2f getSize() const;  // 当前缩放后的尺寸
    sf::FloatRect getBounds() const; // 点击检测/布局用

    bool isJoker() const { return joker_; }

    void draw(sf::RenderWindow& win) const;

private:
    void updateTexture();

    Card card_{};  // A 成员的牌对象
    bool joker_ = false;
    bool faceUp_ = true;
    float scale_ = 1.f;
    sf::Sprite sprite_;
    sf::RectangleShape placeholder_;  // 无贴图时的占位块
};
