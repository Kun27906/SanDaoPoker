#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

// 四态贴图按钮(正常/悬停/按下/禁用) + 居中文字 + 点击回调
//   贴图: assets/ui/buttons/btn_{normal,hover,pressed,disabled}.png (由 AssetManager 统一加载)
//   选中态 setSelected(true): 常驻"按下"贴图且不恢复 —— 选房/难度/道选择的"已选中"提示
//   禁用态 setDisabled(true): 灰色贴图(仅外观; 仍可点击, 由回调决定行为, 如弹"余额不足"提示)
//   着色   setTint(色):       贴图整体着色(默认白=原色); 两段确认的警告态用红色
// 用法:
//   Button btn("Start", {100,100}, {200,50});
//   btn.setCallback([](){ /* 点击时执行 */ });
//   主循环里: btn.handleEvent(event, window); btn.draw(window);

class Button {
public:
    Button() = default;
    Button(const std::string& text, const sf::Vector2f& pos, const sf::Vector2f& size);

    void setText(const std::string& t);
    void setPosition(const sf::Vector2f& p);
    void setSize(const sf::Vector2f& s);
    void setCharacterSize(unsigned size);
    void setCallback(std::function<void()> cb);
    void setSelected(bool s);      // 选中态: 常驻"按下"贴图(不恢复)
    void setDisabled(bool d);      // 禁用外观: 灰色贴图(不拦截点击)
    void setTint(sf::Color c);     // 贴图着色(默认白)

    bool contains(const sf::Vector2f& point) const;

    // 在事件循环中调用:处理悬停高亮 + 左键点击(按下瞬间触发回调)
    void handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    // 非 const:按当前状态切换四态贴图
    void draw(sf::RenderWindow& win);

private:
    void centerText();     // 文字在按钮内居中
    void refreshSprite();  // 按状态取贴图 + 同步位置/缩放/着色

    sf::Sprite sprite_;    // 四态贴图(拉伸到按钮尺寸)
    sf::Text text_;
    std::function<void()> callback_;
    sf::Vector2f pos_{0.f, 0.f};
    sf::Vector2f size_{160.f, 52.f};
    sf::Color tint_ = sf::Color::White;
    bool hovered_ = false;
    bool selected_ = false;
    bool disabled_ = false;
};
