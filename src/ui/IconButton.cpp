#include "ui/IconButton.h"
#include "render/SoundManager.h"

// 窗口物理像素 -> 渲染逻辑坐标(与 Button 一致;高 DPI/缩放下不失真)
static sf::Vector2f mousePos(const sf::RenderWindow& win) {
    return win.mapPixelToCoords(sf::Mouse::getPosition(win));
}

void IconButton::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (!visible_ || !tex_) return;
    if (e.type == sf::Event::MouseMoved) {
        hovered_ = contains(mousePos(win));
    } else if (e.type == sf::Event::MouseButtonPressed &&
               e.mouseButton.button == sf::Mouse::Left) {
        if (contains(mousePos(win))) {
            pressed_ = true;
        }
    } else if (e.type == sf::Event::MouseButtonReleased &&
               e.mouseButton.button == sf::Mouse::Left) {
        if (pressed_) {
            if (contains(mousePos(win))) {
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
