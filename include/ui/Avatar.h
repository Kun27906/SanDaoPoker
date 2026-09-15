#pragma once
#include <SFML/Graphics.hpp>
#include <string>

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
    // 名牌最小宽度(0=仅随昵称自适应); 本人头像可设较大值以预留更长昵称空间
    void setMinPlateWidth(float w) { minPlateW_ = w; }
    // 本人样式: 名牌与圆心共线且昵称居中于可见区; 他人(false): 名牌置于圆环右下角
    void setSelfStyle(bool s) { selfStyle_ = s; }
    sf::FloatRect getBounds() const;                          // 圆环+名牌整体范围(布局用)
    // 昵称名牌区域(最近一次 draw 时记录; 点击名牌改名用)
    sf::FloatRect plateBounds() const { return plateRect_; }
    // 头像圆内点击判定(点击圆换头像用); margin > 0 时外扩判定半径(点击容差)
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
    float minPlateW_ = 0.f;    // 名牌最小宽度(0=纯自适应)
    bool  selfStyle_ = false;  // true=本人(名牌与圆心共线, 昵称居中于可见区)
    sf::FloatRect plateRect_{};  // 最近一次绘制的名牌矩形(plateBounds 返回)
    sf::Text text_;
    bool fontReady_ = false;
};
