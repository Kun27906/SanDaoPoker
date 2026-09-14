#pragma once
#include <SFML/Graphics.hpp>
#include "render/AssetManager.h"

// ====== 弹窗装饰边框: 9 宫格 + 四边平铺绘制 ======
// 素材: assets/ui/table/panel_frame.png (128x128; 四角 48x48 + 四边 32px 宽瓦片 + 中心透明)
// 纹样: 双线绞索结(上下) / 连续方结(左右) / 四角双钱结 + 8 字结扣; 通体金色, 细线 1~2px
// 绘制: 四角 1:1 贴; 四边按瓦片重复平铺(每片仅做微小等比缩放, 避免长距离拉伸导致纹样变形/接缝错位)
// 用法: 画完弹窗底色/描边后调用一次(边框绘制在矩形外沿一圈):
//   panel_frame::draw(win, sf::FloatRect(panel_.getPosition(), panel_.getSize()));
namespace panel_frame {

constexpr float SLICE = 48.f;   // 四角切片(像素, 与素材一致)
constexpr float TILE  = 32.f;   // 四边瓦片宽(像素, 与素材一致)
constexpr float BAND  = 48.f;   // 边框外扩宽度

inline void draw(sf::RenderWindow& win, const sf::FloatRect& rect) {
    const sf::Texture* t = AssetManager::instance().tableTexture("panel_frame");
    if (!t) return;   // 素材缺失: 静默跳过(弹窗仍可用)

    const float texW = static_cast<float>(t->getSize().x);
    const float texH = static_cast<float>(t->getSize().y);
    if (texW <= SLICE * 2.f || texH <= SLICE * 2.f) return;

    const float ox = rect.left - BAND;
    const float oy = rect.top - BAND;
    const float ow = rect.width + BAND * 2.f;
    const float oh = rect.height + BAND * 2.f;

    sf::Sprite sp(*t);

    // ---- 四角: 1:1 ----
    const float csx[4] = {0.f, texW - SLICE, 0.f, texW - SLICE};
    const float csy[4] = {0.f, 0.f, texH - SLICE, texH - SLICE};
    const float cdx[4] = {ox, ox + ow - SLICE, ox, ox + ow - SLICE};
    const float cdy[4] = {oy, oy, oy + oh - SLICE, oy + oh - SLICE};
    for (int i = 0; i < 4; i++) {
        sp.setTextureRect(sf::IntRect(static_cast<int>(csx[i]), static_cast<int>(csy[i]),
                                      static_cast<int>(SLICE), static_cast<int>(SLICE)));
        sp.setScale(1.f, 1.f);
        sp.setPosition(cdx[i], cdy[i]);
        win.draw(sp);
    }

    // ---- 上下边: 横向平铺(瓦片高 48, 宽 32) ----
    const float midY[2] = {0.f, texH - SLICE};
    const float dstY[2] = {oy, oy + oh - SLICE};
    const float lenH = ow - SLICE * 2.f;
    if (lenH > 1.f) {
        const int nH = std::max(1, static_cast<int>(std::lround(lenH / TILE)));
        const float wTile = lenH / static_cast<float>(nH);
        for (int r = 0; r < 2; r++) {
            for (int i = 0; i < nH; i++) {
                sp.setTextureRect(sf::IntRect(static_cast<int>(SLICE), static_cast<int>(midY[r]),
                                              static_cast<int>(TILE), static_cast<int>(SLICE)));
                sp.setScale(wTile / TILE, 1.f);
                sp.setPosition(ox + SLICE + wTile * static_cast<float>(i), dstY[r]);
                win.draw(sp);
            }
        }
    }

    // ---- 左右边: 纵向平铺(瓦片宽 48, 高 32) ----
    const float midX[2] = {0.f, texW - SLICE};
    const float dstX[2] = {ox, ox + ow - SLICE};
    const float lenV = oh - SLICE * 2.f;
    if (lenV > 1.f) {
        const int nV = std::max(1, static_cast<int>(std::lround(lenV / TILE)));
        const float hTile = lenV / static_cast<float>(nV);
        for (int c = 0; c < 2; c++) {
            for (int i = 0; i < nV; i++) {
                sp.setTextureRect(sf::IntRect(static_cast<int>(midX[c]), static_cast<int>(SLICE),
                                              static_cast<int>(SLICE), static_cast<int>(TILE)));
                sp.setScale(1.f, hTile / TILE);
                sp.setPosition(dstX[c], oy + SLICE + hTile * static_cast<float>(i));
                win.draw(sp);
            }
        }
    }
}

} // namespace panel_frame
