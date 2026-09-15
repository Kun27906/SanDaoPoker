#pragma once

// 窗口与牌面基准尺寸
namespace layout {

inline constexpr unsigned WINDOW_W = 1280;
inline constexpr unsigned WINDOW_H = 800;
inline constexpr float CARD_UNIT_W = 200.f;  // 名称避开 core/Card.h 的宏
inline constexpr float CARD_UNIT_H = 280.f;

}
