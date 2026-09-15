#pragma once
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <string>

// 提供全局唯一的默认字体
// 加载顺序: 项目自带字体 -> Windows 系统字体目录, 系统目录由 WINDIR 环境变量定位

namespace font_util {

inline std::string systemFontsDir() {
    char* buf = nullptr;
    size_t len = 0;
    if (_dupenv_s(&buf, &len, "WINDIR") != 0 || buf == nullptr) {
        return {};
    }
    std::string dir(buf);
    free(buf);
    return dir + "/Fonts/";
}

inline const sf::Font& defaultFont() {
    static sf::Font font;
    static bool loaded = false;
    if (!loaded) {
        loaded = true;
        const std::string sysFonts = systemFontsDir();
        const std::string candidates[] = {
            "assets/fonts/SourceHanSansSC-Regular.otf",
            "assets/fonts/font.ttf",
            "assets/fonts/msyh.ttf",
            sysFonts + "msyh.ttc",
            sysFonts + "arial.ttf"
        };
        for (const std::string& path : candidates) {
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
