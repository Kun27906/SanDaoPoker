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

const sf::Color C_FRAME_BG(30, 36, 56);       // 名牌深色底
const sf::Color C_FRAME_EDGE(150, 170, 220);  // 边框/描边亮色
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
    float border = std::max(3.f, r_ * 0.12f);
    float outer = r_ + border / 2.f;
    float padX = cs_ * 0.7f;
    float estW = std::max(2.f * cs_ + padX * 2.f, minPlateW_);
    float plateH = cs_ * 1.8f;
    float top = c_.y - std::max(outer, plateH / 2.f);
    float bottom = c_.y + std::max(outer, plateH / 2.f);
    float left = c_.x - outer;
    float right = c_.x + r_ * 0.9f + estW;
    return sf::FloatRect(left, top, right - left, bottom - top);
}

void Avatar::draw(sf::RenderWindow& win) {
    if (!fontReady_) updateText();
    const float cx = c_.x, cy = c_.y, r = r_;
    const float border = std::max(3.f, r * 0.12f);   // 边框厚度
    const float outer = r + border / 2.f;            // 边框外缘半径

    // ---- 1. 名牌(最下层: 先画 → 被头像盖住的左端自然被裁掉, 不遮挡头像内容) ----
    updateText();
    text_.setString(str_util::utf8(name_.c_str()));
    sf::FloatRect tb = text_.getLocalBounds();
    float padX = cs_ * 0.7f;
    float plateW = tb.width + padX * 2.f;            // 自适应字数
    if (plateW < minPlateW_) plateW = minPlateW_;    // 本人: 预留更长昵称空间
    float plateH = cs_ * 1.8f;
    float px, py;
    if (selfStyle_) {
        px = cx + r * 0.9f;         // 本人: 水平轴与圆心共线
        py = cy - plateH / 2.f;
    } else {
        px = cx + r * 0.35f;        // 他人: 圆环右下角
        py = cy + r * 0.30f;
    }
    roundedRect(win, px, py, plateW, plateH, plateH * 0.28f, C_FRAME_BG, C_FRAME_EDGE);

    // ---- 2. 圆形头像(素材已按圆裁剪): 直径 2*outer, 完整填满圆环内侧, 无缝隙 ----
    float d = outer * 2.f;
    if (tex_ && tex_->getSize().x > 0) {
        sf::Sprite sp(*tex_);
        float s = d / static_cast<float>(tex_->getSize().x);
        sp.setScale(s, s);
        sp.setPosition(cx - outer, cy - outer);
        win.draw(sp);
    } else {
        sf::CircleShape ph(outer);
        ph.setPosition(cx - outer, cy - outer);
        ph.setFillColor(sf::Color(52, 60, 86));
        win.draw(ph);
    }

    // ---- 3. 圆环边框(叠在头像外缘之上, 压住裁剪边缘) ----
    sf::CircleShape ring(r);
    ring.setPosition(cx - r, cy - r);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineColor(C_FRAME_EDGE);
    ring.setOutlineThickness(border);
    win.draw(ring);

    // ---- 4. 昵称(最后画): 居中于"未被头像遮挡"的可见区, 不与边框重叠 ----
    float visLeft = cx + outer;
    if (visLeft < px) visLeft = px;
    float visW = px + plateW - visLeft;
    if (visW < tb.width) visW = tb.width;            // 极端情况兜底
    float tx = visLeft + (visW - tb.width) / 2.f - tb.left;
    text_.setPosition(tx, py + (plateH - tb.height) / 2.f - tb.top);
    win.draw(text_);
}
