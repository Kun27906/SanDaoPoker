#include "ui/ChipBar.h"
#include "ui/FontUtil.h"
#include "render/AssetManager.h"
#include <cstdio>

namespace {
constexpr float W = 250.f;
constexpr float H = 48.f;
}

ChipBar::ChipBar() {
    text_.setFont(font_util::defaultFont());
    text_.setCharacterSize(24);
    text_.setFillColor(sf::Color(255, 215, 0));
    label_.setFont(font_util::defaultFont());
    label_.setCharacterSize(15);
    label_.setFillColor(sf::Color(255, 240, 200));
}

void ChipBar::setPosition(const sf::Vector2f& pos) {
    pos_ = pos;
}

int ChipBar::pickChipIdx(int balance) const {
    int a = balance < 0 ? -balance : balance;
    if (a >= 100) return 4;
    if (a >= 50) return 3;
    if (a >= 10) return 2;
    if (a >= 5) return 1;
    return 0;
}

void ChipBar::draw(sf::RenderWindow& win, int balance) {
    sf::FloatRect box(pos_, sf::Vector2f(W, H));

    // 1. 渐变背景(上深蓝->下亮蓝紫, 垂直渐变)
    sf::VertexArray grad(sf::Quads, 4);
    grad[0].position = sf::Vector2f(box.left, box.top);
    grad[1].position = sf::Vector2f(box.left + box.width, box.top);
    grad[2].position = sf::Vector2f(box.left + box.width, box.top + box.height);
    grad[3].position = sf::Vector2f(box.left, box.top + box.height);
    grad[0].color = sf::Color(20, 40, 90, 235);
    grad[1].color = sf::Color(20, 40, 90, 235);
    grad[2].color = sf::Color(70, 110, 200, 235);
    grad[3].color = sf::Color(70, 110, 200, 235);
    win.draw(grad);

    // 2. 外边框(亮金)
    sf::RectangleShape border(sf::Vector2f(box.width, box.height));
    border.setPosition(box.left, box.top);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(255, 215, 0));
    border.setOutlineThickness(2.f);
    win.draw(border);

    // 3. 左端筹码图标(约 36x36, 居中于左端)
    int idx = pickChipIdx(balance);
    if (const sf::Texture* t = AssetManager::instance().chipTexture(idx)) {
        sf::Sprite chip(*t);
        float scale = 36.f / t->getSize().x;
        chip.setScale(scale, scale);
        chip.setPosition(box.left + 6.f, box.top + (box.height - 36.f) / 2.f);
        win.draw(chip);
    }

    // 4. 分隔竖线(数字与图标分开)
    sf::RectangleShape sep(sf::Vector2f(2.f, box.height - 14.f));
    sep.setPosition(box.left + 52.f, box.top + 7.f);
    sep.setFillColor(sf::Color(255, 255, 255, 160));
    win.draw(sep);

    // 5. 数字
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%d", balance);
    if (balance != shownBalance_) {
        text_.setString(str_util::utf8(buf));
        shownBalance_ = balance;
    }
    text_.setPosition(box.left + 64.f, box.top + 4.f);
    label_.setString(str_util::utf8("筹码"));
    label_.setPosition(box.left + 64.f, box.top + 28.f);
    win.draw(label_);
    win.draw(text_);
}
