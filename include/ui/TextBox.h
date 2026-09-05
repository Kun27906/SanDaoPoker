#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ====== TextBox 文本显示控件 ======
// 功能:一行(或多行,支持 \n)文字的显示封装(字体/字号/颜色/位置/对齐)
// 用法:
//   TextBox t("Hello", {100,100}, 28);
//   t.setColor(sf::Color::White);
//   t.centerOrigin();        // 进入"居中模式":之后 setPosition 以文字中心为准,
//                            // 且任何 setText/setCharacterSize 都会自动保持居中
//   t.draw(window);

class TextBox {
public:
    TextBox() = default;
    TextBox(const std::string& text, const sf::Vector2f& pos, unsigned size = 24);

    void setText(const std::string& t);
    void setPosition(const sf::Vector2f& p);
    void setCharacterSize(unsigned s);
    void setColor(const sf::Color& c);
    void centerOrigin();   // 进入居中模式(自动保持居中)

    sf::FloatRect getBounds() const { return text_.getLocalBounds(); }
    const sf::String& getString() const { return text_.getString(); }

    void draw(sf::RenderWindow& win) const;

private:
    void recenter();   // 按当前文本重新计算居中原点

    sf::Text text_;
    bool centered_ = false;   // 是否处于居中模式
};
