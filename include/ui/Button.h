#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

// ====== Button 通用按钮控件 ======
// 功能:矩形按钮 + 居中文字 + 三态颜色(普通/悬停/按下) + 点击回调
// 用法:
//   Button btn("Start", {100,100}, {200,50});
//   btn.setCallback([](){ /* 点击时执行 */ });
//   主循环里: btn.handleEvent(event, window); btn.draw(window);

class Button {
public:
    Button() = default;
    Button(const std::string& text, const sf::Vector2f& pos, const sf::Vector2f& size);

    // ---- 属性设置 ----
    void setText(const std::string& t);
    void setPosition(const sf::Vector2f& p);
    void setSize(const sf::Vector2f& s);
    void setCharacterSize(unsigned size);
    void setCallback(std::function<void()> cb);
    void setColors(sf::Color normal, sf::Color hover, sf::Color pressed);

    // ---- 查询 ----
    bool contains(const sf::Vector2f& point) const;
    bool isHovered() const { return hovered_; }

    // ---- 交互与绘制 ----
    // 在事件循环中调用:处理悬停高亮 + 左键点击(按下瞬间触发回调)
    void handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    // 非 const:绘制时按悬停状态切换颜色
    void draw(sf::RenderWindow& win);

private:
    void centerText();  // 文字在按钮内居中

    sf::RectangleShape rect_;
    sf::Text text_;
    sf::Color normal_{64, 120, 200};
    sf::Color hover_{90, 160, 240};
    sf::Color pressed_{40, 85, 150};
    std::function<void()> callback_;
    bool hovered_ = false;
};
