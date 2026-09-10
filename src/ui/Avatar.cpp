#include "ui/Avatar.h"
#include "ui/FontUtil.h"
#include <algorithm>

namespace {
// 圆角矩形(1 矩形 + 1 窄矩形 + 4 圆拼出); 先画描边再画填充
void roundedRect(sf::RenderWindow& win, float x, float y, float w, float h,
                 float rad, sf::Color fill, sf::Color outline) {
    auto paint = [&](float ox, float oy, float ow, float oh, float orad, sf::Color col) {
        sf::RectangleShape a(sf::Vector2f(ow - 2 * orad, oh));
        a.setPosition(ox + orad, oy);
        a.setFillColor(col);
        win.draw(a);
        sf::RectangleShape b(sf::Vector2f(ow, oh - 2 * orad));
        b.setPosition(ox, oy + orad);
        b.setFillColor(col);
        win.draw(b);
        for (int i = 0; i < 4; i++) {
            sf::CircleShape cc(orad);
            cc.setFillColor(col);
            float ccx = (i % 2 == 0) ? ox + orad : ox + ow - orad;
            float ccy = (i < 2) ? oy + orad : oy + oh - orad;
            cc.setPosition(ccx - orad, ccy - orad);
            win.draw(cc);
        }
    };
    paint(x - 2.f, y - 2.f, w + 4.f, h + 4.f, rad + 2.f, outline);
    paint(x, y, w, h, rad, fill);
}

// 头像图方形边长系数: 令方形为"内接圆"的最大方形 → 四角恰被边框覆盖
// 方形半对角线 = (SIDE_FACTOR/2)*sqrt(2) ≈ 0.99*r < r(外圈半径) → 不露出圈外
constexpr float SIDE_FACTOR = 1.40f;
constexpr float RING_FACTOR = 0.30f;   // 边框厚度 = 0.30*r(足以遮挡方形四角)
}

void Avatar::updateText() {
    if (!fontReady_) {
        text_.setFont(font_util::defaultFont());
        fontReady_ = true;
    }
    cs_ = std::max(12.f, r_ * 0.62f);          // 字体随半径增大
    text_.setCharacterSize(static_cast<unsigned>(cs_));
    text_.setFillColor(sf::Color(240, 245, 255));
}

sf::FloatRect Avatar::getBounds() const {
    float padX = cs_ * 0.7f;
    float estW = std::max(2.f * cs_ + padX * 2.f, minPlateW_);
    float plateH = cs_ * 1.8f;
    float left = c_.x - r_;
    float top = c_.y - std::max(r_, plateH / 2.f);
    float right = c_.x + r_ * 0.45f + estW;
    float bottom = c_.y + std::max(r_, plateH / 2.f);
    return sf::FloatRect(left, top, right - left, bottom - top);
}

void Avatar::draw(sf::RenderWindow& win) {
    if (!fontReady_) updateText();
    const float cx = c_.x, cy = c_.y, r = r_;
    const float t = r * RING_FACTOR;           // 边框厚度
    const float side = r * SIDE_FACTOR;        // 头像图边长(=内接圆直径的放大, 填满内圈)

    // 1. 深色圆底(兜底, 保证圆外不露任何东西)
    sf::CircleShape base(r);
    base.setPosition(cx - r, cy - r);
    base.setFillColor(sf::Color(30, 36, 56));
    win.draw(base);

    // 2. 头像图: 方形填满内圈(内接圆口径); 四角由下一圈粗边框遮挡
    sf::Vector2f ip(cx - side / 2.f, cy - side / 2.f);
    if (tex_ && tex_->getSize().x > 0) {
        sf::Sprite sp(*tex_);
        float s = side / static_cast<float>(tex_->getSize().x);
        sp.setScale(s, s);
        sp.setPosition(ip);
        win.draw(sp);
    } else {
        sf::RectangleShape ph(sf::Vector2f(side, side));
        ph.setPosition(ip);
        ph.setFillColor(sf::Color(52, 60, 86));
        win.draw(ph);
    }

    // 3. 粗圆环边框: 外缘 = r, 内缘 = r-t (覆盖方形四角, 圈外不露)
    sf::CircleShape ring(r - t / 2.f);
    ring.setPosition(cx - (r - t / 2.f), cy - (r - t / 2.f));
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineColor(sf::Color(150, 170, 220));
    ring.setOutlineThickness(t);
    win.draw(ring);

    // 4. 名牌(右接圆环; 水平轴与圆心共线; 宽度随昵称字数自适应)
    updateText();
    text_.setString(str_util::utf8(name_.c_str()));
    sf::FloatRect tb = text_.getLocalBounds();
    float padX = cs_ * 0.7f;
    float plateW = tb.width + padX * 2.f;      // 自适应字数多少
    if (plateW < minPlateW_) plateW = minPlateW_;   // 本人头像可预留更长昵称空间
    float plateH = cs_ * 1.8f;
    float px = cx + r * 0.45f;
    float py = cy - plateH / 2.f;              // ← 垂直居中(与圆心同一水平轴)
    roundedRect(win, px, py, plateW, plateH, plateH * 0.28f,
                sf::Color(30, 36, 56), sf::Color(150, 170, 220));
    text_.setPosition(px + padX - tb.left,
                      py + (plateH - tb.height) / 2.f - tb.top);
    win.draw(text_);
}
