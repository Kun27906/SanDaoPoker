#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// 一行文字封装; centerOrigin 之后 setPosition 以文字中心为准
class TextBox {
public:
    TextBox() = default;
    TextBox(const std::string& text, const sf::Vector2f& pos, unsigned size = 24);

    void setText(const std::string& t);
    void setPosition(const sf::Vector2f& p);
    void setCharacterSize(unsigned s);
    void setColor(const sf::Color& c);
    void centerOrigin();

    sf::FloatRect getBounds() const { return text_.getLocalBounds(); }

    void draw(sf::RenderWindow& win) const;

private:
    void recenter();

    sf::Text text_;
    bool centered_ = false;
};
