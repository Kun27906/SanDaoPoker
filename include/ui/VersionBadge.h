#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// ====== VersionBadge 左下角版本号(+ 可点击展开版本历史) ======
// 左下角显示 GAME_VERSION(来源 core/VersionInfo.h)。
// clickable=true 时单击可打开"版本历史"弹窗:
//   按时间顺序(从早到晚)逐条列出 版本号 + 主要更新;
//   支持鼠标滚轮上下翻动(内容超出时显示滚动条); 右上角圆圈+close 关闭。
// 用法(场景内):
//   if (versionBadge_.handleEvent(e, win)) return;   // 弹窗打开时消费事件
//   ...
//   versionBadge_.draw(win);
class VersionBadge {
public:
    explicit VersionBadge(bool clickable);

    void setPosition(const sf::Vector2f& p);   // 文字左上角(左下角区域)
    // 返回 true 表示事件已被弹窗消费(调用方应直接 return)
    bool handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void draw(sf::RenderWindow& win);
    bool isOpen() const { return open_; }

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

    // ---- 弹窗 ----
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
