#include "ui/ChipBar.h"
#include "ui/FontUtil.h"
#include "render/AssetManager.h"
#include "render/SoundManager.h"
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

// 立即设定显示值(无动画)
void ChipBar::setImmediate(int v) {
    shown_ = v;
    from_ = v;
    target_ = v;
    rolling_ = false;
    t_ = 0.f;
    inited_ = true;
}

// 从当前显示值滚动到 v(缓出: 先快后慢, 像计数器)
void ChipBar::rollTo(int v, float seconds) {
    if (!inited_) { setImmediate(v); return; }
    if (v == shown_) { target_ = v; rolling_ = false; return; }
    from_ = shown_;
    target_ = v;
    t_ = 0.f;
    dur_ = seconds > 0.05f ? seconds : 0.05f;
    rolling_ = true;
}

void ChipBar::update(float dt) {
    if (!rolling_) return;
    t_ += dt / dur_;
    if (t_ >= 1.f) {
        t_ = 1.f;
        rolling_ = false;
        shown_ = target_;
        return;
    }
    float e = 1.f - (1.f - t_) * (1.f - t_);   // 二次缓出
    shown_ = from_ + static_cast<int>((target_ - from_) * e);
}

// 点击左端筹码图标 -> 播放 chip 音效(筹码图案素材可见处均可点)
bool ChipBar::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (e.type != sf::Event::MouseButtonPressed ||
        e.mouseButton.button != sf::Mouse::Left) {
        return false;
    }
    sf::Vector2f mp = win.mapPixelToCoords(
        sf::Vector2i(e.mouseButton.x, e.mouseButton.y));
    sf::FloatRect iconRect(pos_.x + 2.f, pos_.y + 2.f, 44.f, 44.f);
    if (iconRect.contains(mp)) {
        SoundManager::instance().playChip();
        return true;
    }
    return false;
}

void ChipBar::draw(sf::RenderWindow& win) {
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

    // 2. 外边框(亮金; 滚动中加亮提示)
    sf::RectangleShape border(sf::Vector2f(box.width, box.height));
    border.setPosition(box.left, box.top);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(rolling_ ? sf::Color(255, 245, 160) : sf::Color(255, 215, 0));
    border.setOutlineThickness(rolling_ ? 3.f : 2.f);
    win.draw(border);

    // 3. 左端筹码图标(按当前显示值所在区间档位取素材; 滚动时图标随数字换档)
    int shownForIcon = rolling_ ? shown_ : target_;
    if (const sf::Texture* t = AssetManager::instance().chipForAmount(shownForIcon)) {
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

    // 5. 数字(滚动中显示中间值)
    if (shown_ != cachedShown_) {
        char buf[24];
        std::snprintf(buf, sizeof(buf), "%d", shown_);
        text_.setString(str_util::utf8(buf));
        cachedShown_ = shown_;
    }
    text_.setPosition(box.left + 64.f, box.top + 4.f);
    label_.setString(str_util::utf8("筹码"));
    label_.setPosition(box.left + 64.f, box.top + 28.f);
    win.draw(label_);
    win.draw(text_);
}
