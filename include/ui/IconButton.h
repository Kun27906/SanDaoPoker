#pragma once
#include <SFML/Graphics.hpp>
#include <functional>

// ====== IconButton 图标按钮控件(成员C, 阶段9) ======
// 用一张方形贴图做按钮(图标素材 100x100 缩放到按钮尺寸),
// 无文字; hover 时图标下方衬一圈半透明白底突出; 按下瞬间触发回调。
// 用法:
//   IconButton b;
//   b.setTexture(AssetManager::instance().icon("menuList"));
//   b.setPosition({12,12}); b.setSize(40.f);
//   b.setCallback([](){ ... });
//   b.handleEvent(event, window); b.draw(window);
class IconButton {
public:
    IconButton() = default;

    // 贴图(不持有, 由 AssetManager 保管); nullptr 则不画
    void setTexture(const sf::Texture* t) { tex_ = t; }
    void setPosition(const sf::Vector2f& p) { pos_ = p; }
    sf::Vector2f getPosition() const { return pos_; }
    void setSize(float size) { size_ = size; }   // 正方形边长(像素)
    void setCallback(std::function<void()> cb) { callback_ = std::move(cb); }
    // 是否可见(不可见时不参与事件与绘制)
    void setVisible(bool v) { visible_ = v; }
    bool isVisible() const { return visible_; }

    bool contains(const sf::Vector2f& point) const {
        return point.x >= pos_.x && point.x <= pos_.x + size_ &&
               point.y >= pos_.y && point.y <= pos_.y + size_;
    }

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void draw(sf::RenderWindow& win);

private:
    const sf::Texture* tex_ = nullptr;
    sf::Vector2f pos_{0.f, 0.f};
    float size_ = 40.f;
    bool visible_ = true;
    bool hovered_ = false;
    bool pressed_ = false;
    std::function<void()> callback_;
};
