#include "render/CardSprite.h"
#include "render/AssetManager.h"
#include "render/Layout.h"

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
    placeholder_.setSize(getSize());
}

sf::Vector2f CardSprite::getSize() const {
    // 尺寸取自当前贴图, 未加载时用基准值
    if (const sf::Texture* t = sprite_.getTexture()) {
        return sf::Vector2f(static_cast<float>(t->getSize().x) * scale_,
                            static_cast<float>(t->getSize().y) * scale_);
    }
    return sf::Vector2f(layout::CARD_UNIT_W * scale_, layout::CARD_UNIT_H * scale_);
}

sf::FloatRect CardSprite::getBounds() const {
    return sf::FloatRect(sprite_.getPosition(), getSize());
}

void CardSprite::updateTexture() {
    const AssetManager& am = AssetManager::instance();
    const sf::Texture* t = nullptr;
    int bi = am.currentBack();
    if (faceUp_) {
        // 大小王走 Jokers 贴图(cardTexture 已支持);加载失败时用牌背兜底
        t = am.cardTexture(card_.getSuit(), card_.getRank());
        if (!t) t = am.backTexture(bi);
    } else {
        t = am.backTexture(bi);
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
