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

const sf::Color C_FRAME_BG(30, 36, 56);      // 边框/名牌深色底
const sf::Color C_FRAME_EDGE(150, 170, 220);// 边框亮边

constexpr float OUTER_FACTOR = 1.45f;   // 圆环外缘 = 1.45r: 足够覆盖 2r 方形头像的四角(1.414r)
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
    float outer = r_ * OUTER_FACTOR;
    float top = c_.y - std::max(outer, plateH / 2.f);
    float bottom = c_.y + std::max(outer, plateH / 2.f);
    float left = c_.x - outer;
    float right = c_.x + r_ * 1.05f + estW;
    return sf::FloatRect(left, top, right - left, bottom - top);
}

void Avatar::draw(sf::RenderWindow& win) {
    if (!fontReady_) updateText();
    const float cx = c_.x, cy = c_.y, r = r_;
    const float outer = r * OUTER_FACTOR;   // 圆环外缘(覆盖方形四角)
    const float side = r * 2.f;             // 头像图 = 外圈直径 → 完整"填充"半径 r 的圆

    // 1. 头像图: 2r 方形, 恰好填满半径 r 的圆(四角随后由圆环压住, 圈外不露)
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

    // 2. 名牌(先画 → 左端被圆环压住)
    updateText();
    text_.setString(str_util::utf8(name_.c_str()));
    sf::FloatRect tb = text_.getLocalBounds();
    float padX = cs_ * 0.7f;
    float plateW = tb.width + padX * 2.f;              // 自适应字数
    if (plateW < minPlateW_) plateW = minPlateW_;      // 本人: 预留更长昵称空间
    float plateH = cs_ * 1.8f;
    float px, py;
    if (selfStyle_) {
        px = cx + r * 1.05f;        // 左端伸入圆环带 → 被圆环压住一部分
        py = cy - plateH / 2.f;     // 水平轴与圆心共线
    } else {
        px = cx + r * 0.45f;        // 他人: 圆环右下角
        py = cy + r * 0.28f;
    }
    roundedRect(win, px, py, plateW, plateH, plateH * 0.28f, C_FRAME_BG, C_FRAME_EDGE);

    // 3. 圆环(后画 → 压住头像四角 + 压住名牌左端)
    {
        float mid = (r + outer) / 2.f;
        float thick = outer - r;
        sf::CircleShape ring(mid);
        ring.setPosition(cx - mid, cy - mid);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineColor(C_FRAME_BG);
        ring.setOutlineThickness(thick);
        win.draw(ring);
        sf::CircleShape edgeLine(outer);
        edgeLine.setPosition(cx - outer, cy - outer);
        edgeLine.setFillColor(sf::Color::Transparent);
        edgeLine.setOutlineColor(C_FRAME_EDGE);
        edgeLine.setOutlineThickness(std::max(2.f, r * 0.10f));
        win.draw(edgeLine);
    }

    // 4. 昵称(最后画, 永不被遮挡): 本人居中于"圆环外缘之后的可见区", 他人在名牌内居中
    float tx;
    if (selfStyle_) {
        float visLeft = cx + outer;                      // 可见区起点(圆环外缘)
        if (visLeft < px) visLeft = px;
        tx = visLeft + (px + plateW - visLeft) / 2.f - tb.width / 2.f - tb.left;
    } else {
        tx = px + (plateW - tb.width) / 2.f - tb.left;
    }
    text_.setPosition(tx, py + (plateH - tb.height) / 2.f - tb.top);
    win.draw(text_);
}
