#pragma once
#include <SFML/Graphics.hpp>

// ====== ChipBar 筹码显示条(成员C) ======
// 右上角长条形筹码框:
//   左端筹码图标(按余额所在档位区间取素材, 见 AssetManager::chipForAmount)
//   + 竖分隔线 + 数字; 背景用渐变色块(上下两色)以凸显与游戏背景的区别
class ChipBar {
public:
    ChipBar();
    void setPosition(const sf::Vector2f& pos);
    void draw(sf::RenderWindow& win, int balance);

private:
    sf::Vector2f pos_;
    sf::Text text_;        // 数字文本
    sf::Text label_;       // "筹码" 小标签
    int shownBalance_ = -1;
};
