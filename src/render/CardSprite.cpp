#include "render/CardSprite.h"
#include "render/AssetManager.h"

void CardSprite::setCard(const Card& card) {
    card_ = card;
    joker_ = card.isJoker();
    updateTexture();
}

void CardSprite::setPosition(const sf::Vector2f& p) {
    sprite_.setPosition(p);
    placeholder_.setPosition(p);
}

void CardSprite::setScale(float s) {
    scale_ = s;
    sprite_.setScale(s, s);
    sf::Vector2f sz(200.f * s, 280.f * s);
    placeholder_.setSize(sz);
}

sf::Vector2f CardSprite::getSize() const {
    return sf::Vector2f(200.f * scale_, 280.f * scale_);
}

sf::FloatRect CardSprite::getBounds() const {
    return sf::FloatRect(sprite_.getPosition(), getSize());
}

bool CardSprite::hasTexture() const {
    const AssetManager& am = AssetManager::instance();
    const sf::Texture* t = nullptr;
    if (faceUp_) {
        t = joker_ ? nullptr : am.cardTexture(card_.getSuit(), card_.getRank());
    } else {
        t = am.backTexture(backIndex_);
    }
    return t != nullptr;
}

void CardSprite::updateTexture() {
    const AssetManager& am = AssetManager::instance();
    const sf::Texture* t = nullptr;
    if (faceUp_) {
        // 大小王走 Jokers 贴图(cardTexture 已支持);加载失败时用牌背兜底
        t = am.cardTexture(card_.getSuit(), card_.getRank());
        if (!t) t = am.backTexture(backIndex_);
    } else {
        t = am.backTexture(backIndex_);
    }
    if (t) {
        sprite_.setTexture(*t, true);
        placeholder_ = sf::RectangleShape();
    } else {
        // 无贴图:占位块(浅灰 + 后续画 "?" 由调用方处理)
        sprite_ = sf::Sprite();
        placeholder_.setFillColor(sf::Color(120, 120, 120));
        placeholder_.setOutlineColor(sf::Color(200, 200, 200));
        placeholder_.setOutlineThickness(2.f);
    }
}

void CardSprite::draw(sf::RenderWindow& win) const {
    // 每次绘制前同步纹理(素材可能后续才加载,保持兼容)
    const_cast<CardSprite*>(this)->updateTexture();
    if (sprite_.getTexture()) {
        win.draw(sprite_);
    } else {
        win.draw(placeholder_);
    }
}
