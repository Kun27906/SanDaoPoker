#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// 提供全局唯一的默认字体。
// 加载顺序:
// 1) 项目自带字体 assets/fonts/SourceHanSansSC-Regular.otf
// 3) 系统中文字体 微软雅黑 C:/Windows/Fonts/msyh.ttc
// 4) 系统英文字体 Arial

namespace font_util {
inline const sf::Font& defaultFont() {
    static sf::Font font;
    static bool loaded = false;
    if (!loaded) {
        loaded = true;
        const char* candidates[] = {
            "assets/fonts/SourceHanSansSC-Regular.otf",  // 思源黑体
            "assets/fonts/font.ttf",
            "assets/fonts/msyh.ttf",
            "C:/Windows/Fonts/msyh.ttc",                 // 系统中文字体
            "C:/Windows/Fonts/arial.ttf"                 // 系统英文字体
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

// 中文会乱码。所有界面文字必须经此函数显式转换。
namespace str_util {

inline sf::String utf8(const char* s) {
    std::string str(s);
    return sf::String::fromUtf8(str.begin(), str.end());
}

} // namespace str_util
