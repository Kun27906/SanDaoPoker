#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ====== 字体工具 ======
// 提供全局唯一的默认字体(懒加载单例)。
// 加载顺序:
//   1) 项目自带的字体 assets/fonts/ (由 D 成员提供,如思源黑体,支持中文)
//   2) 系统中文字体 微软雅黑 C:/Windows/Fonts/msyh.ttc (开发期兜底,支持中文)
//   3) 系统英文字体 Arial (最后兜底,仅支持英文)
// 说明:SFML 不自带字体,加载失败时文字将无法显示,因此做了多级兜底。

namespace font_util {

inline const sf::Font& defaultFont() {
    static sf::Font font;
    static bool loaded = false;
    if (!loaded) {
        loaded = true;
        const char* candidates[] = {
            "assets/fonts/font.ttf",          // 项目字体(正式)
            "assets/fonts/msyh.ttf",          // 项目字体(微软雅黑拷贝)
            "C:/Windows/Fonts/msyh.ttc",      // 系统中文字体(开发兜底)
            "C:/Windows/Fonts/arial.ttf"      // 系统英文字体(最后兜底)
        };
        for (const char* path : candidates) {
            if (font.loadFromFile(path)) {
                break;
            }
        }
    }
    return font;
}

} // namespace font_util
