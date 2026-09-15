#pragma once
#include <SFML/Graphics.hpp>

// 右上角长条形筹码框:
// 左端筹码图标
// + 竖分隔线 + 数字; 背景用渐变色块以凸显与游戏背景的区别
// 数字跳动动画:
// setImmediate 立即设定显示值
// rollTo 从当前显示值"滚动"到 v
// update 每帧推进滚动
class ChipBar {
public:
    ChipBar();
    void setPosition(const sf::Vector2f& pos);

    void setImmediate(int v);                    // 立即设定
    void rollTo(int v, float seconds = 0.6f);    // 滚动到 v
    void update(float dt);                       // 推进滚动动画
    bool isRolling() const { return rolling_; }

    void draw(sf::RenderWindow& win);            // 用内部显示值绘制

    // 点击左端筹码图标 -> 播放 chip 音效; 返回 true 表示事件已被消费
    bool handleEvent(const sf::Event& e, const sf::RenderWindow& win);

private:
    sf::Vector2f pos_;
    sf::Text text_;        // 数字文本
    sf::Text label_;       // "筹码" 小标签
    int shown_ = 0;        // 当前显示值
    int from_ = 0;         // 动画起点
    int target_ = 0;       // 动画终点
    float t_ = 0.f;        // 动画进度 0..1
    float dur_ = 0.6f;     // 动画时长
    bool rolling_ = false;
    bool inited_ = false;
    int cachedShown_ = -1; // 文本缓存
};
