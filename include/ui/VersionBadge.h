#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// 左下角显示 GAME_VERSION。
// 按时间顺序逐条列出 版本号 + 主要更新;
// 支持鼠标滚轮上下翻动; 右上角圆圈+close 关闭。
// 用法:
// if ) return; // 弹窗打开时消费事件
// ...
// versionBadge_.draw;
class VersionBadge {
public:
    explicit VersionBadge(bool clickable);

    void setPosition(const sf::Vector2f& p);  // 文字左上角
  // 返回 true 表示事件已被弹窗消费
    bool handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void draw(sf::RenderWindow& win);

private:
    void buildEntries();
    float maxScroll() const;
    void clampScroll();

    bool clickable_ = false;
    bool open_ = false;
    bool hovered_ = false;
    float scroll_ = 0.f;
    float contentH_ = 0.f;

    sf::Text badge_;
    sf::FloatRect badgeRect_;

    sf::RectangleShape overlay_;
    sf::RectangleShape panel_;
    sf::RectangleShape listBg_;
    sf::Text title_;
    sf::CircleShape closeRing_;
    sf::Sprite closeIcon_;
    std::vector<sf::Text> verTexts_;
    std::vector<sf::Text> descTexts_;
    sf::FloatRect listRect_;
    sf::FloatRect closeRect_;
    sf::View listView_;
};
