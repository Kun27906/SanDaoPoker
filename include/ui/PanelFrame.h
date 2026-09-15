#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include "render/AssetManager.h"

// 弹窗九宫格装饰边框; 素材缺失时静默跳过
namespace panel_frame {

constexpr float CW = 112.f;
constexpr float CH = 144.f;
constexpr float TW = 32.f;
constexpr float TH = 32.f;
constexpr float SLICE_SCALE = 0.55f;
constexpr float OUTSET = 6.f;

inline void draw(sf::RenderWindow& win, const sf::FloatRect& rect) {
    const sf::Texture* t = AssetManager::instance().tableTexture("panel_frame");
    if (!t) return;

    const float texW = static_cast<float>(t->getSize().x);
    const float texH = static_cast<float>(t->getSize().y);
    if (texW < CW + TW + CW || texH < CH + TH + CH) return;

    // 弹窗过小时按比例缩小角饰
    float s = SLICE_SCALE;
    const float maxS = std::min((rect.width - 8.f) / (2.f * CW), (rect.height - 8.f) / (2.f * CH));
    if (maxS < s) s = std::max(0.15f, maxS);

    const float cw = CW * s, ch = CH * s;
    const float ox = rect.left - OUTSET;
    const float oy = rect.top - OUTSET;
    const float ow = rect.width + OUTSET * 2.f;
    const float oh = rect.height + OUTSET * 2.f;

    sf::Sprite sp(*t);

    const float cx[4] = {0.f, texW - CW, 0.f, texW - CW};
    const float cy[4] = {0.f, 0.f, texH - CH, texH - CH};
    const float dx[4] = {ox, ox + ow - cw, ox, ox + ow - cw};
    const float dy[4] = {oy, oy, oy + oh - ch, oy + oh - ch};
    for (int i = 0; i < 4; i++) {
        sp.setTextureRect(sf::IntRect(static_cast<int>(cx[i]), static_cast<int>(cy[i]),
                                      static_cast<int>(CW), static_cast<int>(CH)));
        sp.setScale(s, s);
        sp.setPosition(dx[i], dy[i]);
        win.draw(sp);
    }

    const float lenH = ow - cw * 2.f;
    if (lenH > 0.5f) {
        for (int r = 0; r < 2; r++) {
            const float sy = (r == 0) ? 0.f : texH - CH;
            const float dyy = (r == 0) ? oy : oy + oh - ch;
            sp.setTextureRect(sf::IntRect(static_cast<int>(CW), static_cast<int>(sy),
                                          static_cast<int>(TW), static_cast<int>(CH)));
            sp.setScale(lenH / TW, s);
            sp.setPosition(ox + cw, dyy);
            win.draw(sp);
        }
    }

    const float lenV = oh - ch * 2.f;
    if (lenV > 0.5f) {
        for (int c = 0; c < 2; c++) {
            const float sx = (c == 0) ? 0.f : texW - CW;
            const float dxx = (c == 0) ? ox : ox + ow - cw;
            sp.setTextureRect(sf::IntRect(static_cast<int>(sx), static_cast<int>(CH),
                                          static_cast<int>(CW), static_cast<int>(TH)));
            sp.setScale(s, lenV / TH);
            sp.setPosition(dxx, oy + ch);
            win.draw(sp);
        }
    }
}

}
