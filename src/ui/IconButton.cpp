#include "ui/IconButton.h"
#include "render/SoundManager.h"

void IconButton::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (!visible_ || !tex_) return;
    if (e.type == sf::Event::MouseMoved) {
        sf::Vector2i mp = sf::Mouse::getPosition(win);
        hovered_ = contains(sf::Vector2f(static_cast<float>(mp.x),
                                         static_cast<float>(mp.y)));
    } else if (e.type == sf::Event::MouseButtonPressed &&
               e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mp = sf::Mouse::getPosition(win);
        if (contains(sf::Vector2f(static_cast<float>(mp.x),
                                  static_cast<float>(mp.y)))) {
            pressed_ = true;
        }
    } else if (e.type == sf::Event::MouseButtonReleased &&
               e.mouseButton.button == sf::Mouse::Left) {
        if (pressed_) {
            sf::Vector2i mp = sf::Mouse::getPosition(win);
            if (contains(sf::Vector2f(static_cast<float>(mp.x),
                                      static_cast<float>(mp.y)))) {
                SoundManager::instance().playClick();
                if (callback_) callback_();
            }
        }
        pressed_ = false;
    }
}

void IconButton::draw(sf::RenderWindow& win) {
    if (!visible_ || !tex_) return;
    const sf::Texture& t = *tex_;
    const float s = size_;
    // hover: 图标下方衬一圈半透明白底(微圆角方形), 提示可点
    if (hovered_) {
        sf::RectangleShape back(sf::Vector2f(s, s));
        back.setPosition(pos_);
        back.setFillColor(sf::Color(255, 255, 255, 46));
        win.draw(back);
    }
    sf::Sprite sp(t);
    float scale = s / static_cast<float>(t.getSize().x);
    sp.setScale(scale, scale);
    sp.setPosition(pos_);
    if (pressed_) {
        // 按下: 轻微缩小提反馈
        sp.setScale(scale * 0.9f, scale * 0.9f);
        sp.setPosition(pos_.x + s * 0.05f, pos_.y + s * 0.05f);
    }
    win.draw(sp);
}
