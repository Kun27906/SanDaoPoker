#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// ====== 字体工具 ======
// 提供全局唯一的默认字体(懒加载单例)。
// 加载顺序:
//   1) 项目自带字体 assets/fonts/SourceHanSansSC-Regular.otf (思源黑体,支持中文)
//   2) 其他候选路径(兼容旧命名)
//   3) 系统中文字体 微软雅黑 C:/Windows/Fonts/msyh.ttc (开发期兜底)
//   4) 系统英文字体 Arial (最后兜底,仅英文)
// 说明:SFML 不自带字体,加载失败时文字将无法显示,因此做了多级兜底。

namespace font_util {
inline const sf::Font& defaultFont() {
    static sf::Font font;
    static bool loaded = false;
    if (!loaded) {
        loaded = true;
        const char* candidates[] = {
            "assets/fonts/SourceHanSansSC-Regular.otf",  // 思源黑体(正式,中文)
            "assets/fonts/font.ttf",                     // 兼容旧命名
            "assets/fonts/msyh.ttf",
            "C:/Windows/Fonts/msyh.ttc",                 // 系统中文字体(开发兜底)
            "C:/Windows/Fonts/arial.ttf"                 // 系统英文字体(最后兜底)
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

// ====== UTF-8 字符串工具 ======
// 注意:SFML 的 sf::String(const char*) 不按 UTF-8 解码(按本地 ANSI 代码页),
//      中文会乱码。所有界面文字必须经此函数显式转换。
namespace str_util {

inline sf::String utf8(const char* s) {
    std::string str(s);
    return sf::String::fromUtf8(str.begin(), str.end());
}

} // namespace str_util
