#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

// 四态贴图按钮; 选中态常驻按下贴图, 禁用态灰色, setTint 用于警告着色
class Button {
public:
    Button() = default;
    Button(const std::string& text, const sf::Vector2f& pos, const sf::Vector2f& size);

    void setText(const std::string& t);
    void setPosition(const sf::Vector2f& p);
    void setSize(const sf::Vector2f& s);
    void setCharacterSize(unsigned size);
    void setCallback(std::function<void()> cb);
    void setSelected(bool s);
    void setDisabled(bool d);
    void setTint(sf::Color c);

    bool contains(const sf::Vector2f& point) const;

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void draw(sf::RenderWindow& win);

private:
    void centerText();
    void refreshSprite();

    sf::Sprite sprite_;
    sf::Text text_;
    std::function<void()> callback_;
    sf::Vector2f pos_{0.f, 0.f};
    sf::Vector2f size_{160.f, 52.f};
    sf::Color tint_ = sf::Color::White;
    bool hovered_ = false;
    bool selected_ = false;
    bool disabled_ = false;
};
