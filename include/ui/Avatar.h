#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ====== Avatar 玩家头像(成员C, 阶段9) ======
// 构成: 深色圆形边框(圆环) + 中间头像位(100x100 方图; 未提供素材时留深色空位)
//       + 右下叠放一个圆角矩形名牌(显示昵称, 深色底 + 亮边)。
// 用法:
//   Avatar a; a.setRadius(26.f); a.setCenter({947,40}); a.setNickname("孤影"); a.draw(win);
//   a.setTexture(tex);   // 头像图(可选, 由 D 提供的 100x100 随机图案)
class Avatar {
public:
    void setNickname(const std::string& n) { name_ = n; }
    void setTexture(const sf::Texture* t) { tex_ = t; }
    void setCenter(const sf::Vector2f& c) { c_ = c; }        // 圆心
    void setRadius(float r) { r_ = r; updateText(); }        // 圆半径(控制整体大小)
    float radius() const { return r_; }
    float charSize() const { return cs_; }
    sf::FloatRect getBounds() const;                          // 圆环+名牌整体范围(布局用)
    void draw(sf::RenderWindow& win);

private:
    void updateText();
    std::string name_;
    const sf::Texture* tex_ = nullptr;
    sf::Vector2f c_{0.f, 0.f};
    float r_ = 26.f;
    float cs_ = 14.f;
    sf::Text text_;
    bool fontReady_ = false;
};
