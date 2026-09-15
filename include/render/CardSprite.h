#pragma once
#include <SFML/Graphics.hpp>
#include "core/Card.h"

// 把核心牌对象显示为牌图; 大小王贴图缺失时用牌背兜底
class CardSprite {
public:
    CardSprite() = default;

    void setCard(const Card& card);
    void setFaceUp(bool up) { faceUp_ = up; }

    void setPosition(const sf::Vector2f& p);
    void setScale(float s);
    sf::Vector2f getSize() const;
    sf::FloatRect getBounds() const;

    bool isJoker() const { return joker_; }

    void draw(sf::RenderWindow& win) const;

private:
    void updateTexture();

    Card card_{};
    bool joker_ = false;
    bool faceUp_ = true;
    float scale_ = 1.f;
    sf::Sprite sprite_;
    sf::RectangleShape placeholder_;
};
