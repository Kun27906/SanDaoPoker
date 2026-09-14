#pragma once
#include <SFML/Graphics.hpp>
#include "render/AssetManager.h"

// ====== 弹窗装饰边框: 9 宫格伸缩绘制 ======
// 素材: assets/ui/table/panel_frame.png (128x128; 四角切片 48x48 固定 1:1, 四边拉伸; 中心透明)
// 纹样: 缠丝双股辫 + 细金丝线 + 内圈珍珠链 + 四角涡卷叶饰(原创程序化绘制)
// 用法: 画完弹窗底色/描边后调用一次, 边框绘制在矩形外沿一圈(外扩 48px)。
//   panel_frame::draw(win, sf::FloatRect(panel_.getPosition(), panel_.getSize()));
namespace panel_frame {

constexpr float SLICE = 48.f;   // 素材切片尺寸(像素, 与素材一致)
constexpr float BAND = 48.f;    // 边框外扩宽度(= 切片尺寸: 四角 1:1 不缩放)

inline void draw(sf::RenderWindow& win, const sf::FloatRect& rect) {
    const sf::Texture* t = AssetManager::instance().tableTexture("panel_frame");
    if (!t) return;   // 素材缺失: 静默跳过(弹窗仍可用)

    const float texW = static_cast<float>(t->getSize().x);
    const float texH = static_cast<float>(t->getSize().y);
    if (texW <= SLICE * 2.f || texH <= SLICE * 2.f) return;

    // 外沿矩形 = 弹窗矩形外扩 BAND
    const float ox = rect.left - BAND;
    const float oy = rect.top - BAND;
    const float ow = rect.width + BAND * 2.f;
    const float oh = rect.height + BAND * 2.f;

    // 三段切分坐标: 素材侧 / 目标侧
    const float sx[4] = {0.f, SLICE, texW - SLICE, texW};
    const float sy[4] = {0.f, SLICE, texH - SLICE, texH};
    const float dx[4] = {ox, ox + SLICE, ox + ow - SLICE, ox + ow};
    const float dy[4] = {oy, oy + SLICE, oy + oh - SLICE, oy + oh};

    sf::Sprite sp(*t);
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            if (r == 1 && c == 1) continue;   // 中心透明, 不绘制
            const float sw = sx[c + 1] - sx[c];
            const float sh = sy[r + 1] - sy[r];
            const float dw = dx[c + 1] - dx[c];
            const float dh = dy[r + 1] - dy[r];
            sp.setTextureRect(sf::IntRect(static_cast<int>(sx[c]), static_cast<int>(sy[r]),
                                          static_cast<int>(sw), static_cast<int>(sh)));
            sp.setScale(dw / sw, dh / sh);
            sp.setPosition(dx[c], dy[r]);
            win.draw(sp);
        }
    }
}

} // namespace panel_frame
