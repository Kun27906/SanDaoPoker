#include "ui/Avatar.h"
#include "ui/FontUtil.h"
#include <algorithm>

namespace {
// 圆角矩形(用 1 个矩形 + 1 个窄矩形 + 4 个圆拼出); 先画描边再画填充
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
    paint(x - 2.f, y - 2.f, w + 4.f, h + 4.f, rad + 2.f, outline);   // 描边
    paint(x, y, w, h, rad, fill);                                    // 填充
}
}

void Avatar::updateText() {
    if (!fontReady_) {
        text_.setFont(font_util::defaultFont());
        fontReady_ = true;
    }
    cs_ = std::max(11.f, r_ * 0.55f);
    text_.setCharacterSize(static_cast<unsigned>(cs_));
    text_.setFillColor(sf::Color(235, 240, 255));
}

sf::FloatRect Avatar::getBounds() const {
    // 粗略范围: 圆环 + 右下名牌(名牌宽按字号估算)
    float estW = 2.f * cs_ + 22.f;
    float left = c_.x - r_;
    float top = c_.y - r_;
    float right = c_.x + r_ * 0.2f + estW;
    float bottom = std::max(c_.y + r_, c_.y + r_ * 0.28f + r_ * 0.9f);
    return sf::FloatRect(left, top, right - left, bottom - top);
}

void Avatar::draw(sf::RenderWindow& win) {
    if (!fontReady_) updateText();
    const float cx = c_.x, cy = c_.y, r = r_;

    // 1. 深色圆底
    sf::CircleShape base(r);
    base.setPosition(cx - r, cy - r);
    base.setFillColor(sf::Color(30, 36, 56));
    win.draw(base);

    // 2. 中间头像位(内接正方形; 有素材画图, 无则深色空位)
    float side = r * 1.42f;
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

    // 3. 圆环边框(深色系亮边)
    sf::CircleShape ring(r);
    ring.setPosition(cx - r, cy - r);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineColor(sf::Color(120, 140, 190));
    ring.setOutlineThickness(3.f);
    win.draw(ring);

    // 4. 名牌(右下叠放, 圆角矩形)
    updateText();
    text_.setString(str_util::utf8(name_.c_str()));
    sf::FloatRect tb = text_.getLocalBounds();
    float plateH = r * 0.9f;
    float plateW = tb.width + 22.f;
    float px = cx + r * 0.2f;
    float py = cy + r * 0.28f;
    roundedRect(win, px, py, plateW, plateH, 7.f,
                sf::Color(30, 36, 56), sf::Color(120, 140, 190));
    text_.setPosition(px + (plateW - tb.width) / 2.f - tb.left,
                      py + (plateH - tb.height) / 2.f - tb.top);
    win.draw(text_);
}
