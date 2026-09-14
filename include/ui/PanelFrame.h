#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include "render/AssetManager.h"

// ====== 弹窗装饰边框: 9 宫格伸缩绘制 ======
// 素材: assets/ui/table/panel_frame.png (256x320; 四角 112x144 + 四边中段 32px, 中心透明)
// 来源: 用户提供的边框图 -> 去白底/去水印 -> 纯金重着色(保留原明暗起伏) -> 切 9 宫格
// 纹样: 四角卷草角饰 + 四边细金线(实测 1px)
// 用法: 画完弹窗底色/描边后调用一次(边框环贴弹窗矩形外沿一圈):
//   panel_frame::draw(win, sf::FloatRect(panel_.getPosition(), panel_.getSize()));
namespace panel_frame {

constexpr float CW = 112.f;   // 角切片宽(素材像素)
constexpr float CH = 144.f;   // 角切片高(素材像素)
constexpr float TW = 32.f;    // 中段切片宽(素材像素)
constexpr float TH = 32.f;    // 中段切片高(素材像素)
constexpr float SLICE_SCALE = 0.55f;   // 角饰缩放(角饰较大, 缩放至贴合弹窗尺度)
constexpr float OUTSET = 6.f;          // 边框外扩(角饰外缘离弹窗边缘的距离)

inline void draw(sf::RenderWindow& win, const sf::FloatRect& rect) {
    const sf::Texture* t = AssetManager::instance().tableTexture("panel_frame");
    if (!t) return;   // 素材缺失: 静默跳过(弹窗仍可用)

    const float texW = static_cast<float>(t->getSize().x);
    const float texH = static_cast<float>(t->getSize().y);
    if (texW < CW + TW + CW || texH < CH + TH + CH) return;

    // 弹窗过小时按比例缩小角饰(保证四角不重叠)
    float s = SLICE_SCALE;
    const float maxS = std::min((rect.width - 8.f) / (2.f * CW), (rect.height - 8.f) / (2.f * CH));
    if (maxS < s) s = std::max(0.15f, maxS);

    const float cw = CW * s, ch = CH * s;
    // 外沿矩形 = 弹窗矩形外扩 OUTSET
    const float ox = rect.left - OUTSET;
    const float oy = rect.top - OUTSET;
    const float ow = rect.width + OUTSET * 2.f;
    const float oh = rect.height + OUTSET * 2.f;

    sf::Sprite sp(*t);

    // ---- 四角: 按 s 缩放(角饰整体等比) ----
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

    // ---- 上/下边中段: 横向拉伸(纵向保持 s, 与角饰厚度一致) ----
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

    // ---- 左/右边中段: 纵向拉伸 ----
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

} // namespace panel_frame
