#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ====== TextBox 文本显示控件 ======
// 功能:一行文字的显示封装(字体/字号/颜色/位置/对齐)
// 用法:
//   TextBox t("Hello", {100,100}, 28);
//   t.setColor(sf::Color::White);
//   t.centerOrigin();        // 原点居中,便于按中心定位
//   t.draw(window);

class TextBox {
public:
    TextBox() = default;
    TextBox(const std::string& text, const sf::Vector2f& pos, unsigned size = 24);

    void setText(const std::string& t);
    void setPosition(const sf::Vector2f& p);
    void setCharacterSize(unsigned size);
    void setColor(const sf::Color& c);

    // 原点设为文字包围盒中心(之后 setPosition 以中心为准)
    void centerOrigin();

    sf::FloatRect getBounds() const { return text_.getLocalBounds(); }
    const sf::String& getString() const { return text_.getString(); }

    void draw(sf::RenderWindow& win) const;

private:
    sf::Text text_;
};
