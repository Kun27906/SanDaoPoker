#pragma once
#include <SFML/Graphics.hpp>
#include "core/Card.h"

// ====== CardSprite 牌精灵(阶段2) ======
// 把 A 成员的核心牌对象(Card)显示为界面上的牌图。
//  - 正面:按 花色/点数 从 AssetManager 取贴图
//  - 牌背:setFaceUp(false) 显示背面
//  - 大小王:贴图未就绪时自动用牌背兜底(等 D 成员提供后自动生效)
// 用法:
//   CardSprite cs;
//   cs.setCard(card);
//   cs.setPosition({100,100});
//   cs.setScale(0.5f);     // 200x280 -> 100x140
//   cs.draw(window);

class CardSprite {
public:
    CardSprite() = default;

    // 设置要显示的牌
    void setCard(const Card& card);
    // 正面/牌背
    void setFaceUp(bool up) { faceUp_ = up; }
    bool isFaceUp() const { return faceUp_; }
    // 牌背颜色:0=红 1=蓝 2=黑(仅牌背时有效)
    void setBackIndex(int idx) { backIndex_ = idx; }

    void setPosition(const sf::Vector2f& p);
    void setScale(float s);          // 整体缩放(原图 200x280)
    sf::Vector2f getSize() const;    // 当前缩放后的尺寸
    sf::FloatRect getBounds() const; // 点击检测/布局用

    bool isJoker() const { return joker_; }
    bool hasTexture() const;         // 当前是否有可用贴图(否则画占位块)

    void draw(sf::RenderWindow& win) const;

private:
    void updateTexture();

    Card card_{};              // A 成员的牌对象
    bool joker_ = false;
    bool faceUp_ = true;
    int backIndex_ = 0;
    float scale_ = 1.f;
    sf::Sprite sprite_;
    sf::RectangleShape placeholder_;  // 无贴图时的占位块
};
