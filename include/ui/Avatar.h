#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// 圆形头像 + 右下昵称名牌
class Avatar {
public:
    void setNickname(const std::string& n) { name_ = n; }
    void setTexture(const sf::Texture* t) { tex_ = t; }
    void setCenter(const sf::Vector2f& c) { c_ = c; }
    void setRadius(float r) { r_ = r; updateText(); }
    // 名牌最小宽度, 本人可设大值预留昵称空间
    void setMinPlateWidth(float w) { minPlateW_ = w; }
    // 本人样式: 名牌与圆心共线; 他人: 名牌置于圆环右下
    void setSelfStyle(bool s) { selfStyle_ = s; }
    sf::FloatRect getBounds() const;
    sf::FloatRect plateBounds() const { return plateRect_; }
    // 圆内点击判定, margin 外扩判定半径
    bool hitCircle(const sf::Vector2f& p, float margin = 0.f) const {
        const float dx = p.x - c_.x, dy = p.y - c_.y;
        const float rr = r_ + margin;
        return (dx * dx + dy * dy) <= rr * rr;
    }
    void draw(sf::RenderWindow& win);

private:
    void updateText();
    std::string name_;
    const sf::Texture* tex_ = nullptr;
    sf::Vector2f c_{0.f, 0.f};
    float r_ = 26.f;
    float cs_ = 14.f;
    float minPlateW_ = 0.f;
    bool  selfStyle_ = false;
    sf::FloatRect plateRect_{};
    sf::Text text_;
    bool fontReady_ = false;
};
