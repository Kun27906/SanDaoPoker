#pragma once
#include "render/SceneManager.h"
#include "render/CardSprite.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include "ui/CountdownBar.h"
#include <array>

// ====== 组牌界面(阶段5: 分三道) ======
// 玩法:9 张手牌点选放入三道(头道/中道/尾道,每道 3 张),
//      选道用左侧三个按钮(当前道高亮),交牌前可重置重摆,
//      倒计时(20秒)归零自动按序摆完并交牌。
// 交牌后:AI 玩家自动随机组牌(阶段8替换为 AIPlayer),切到比牌界面。

class SceneArrange : public Scene {
public:
    explicit SceneArrange(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    void placeCard(int handIdx);   // 把手牌放入当前道空槽
    void resetArrange();           // 清空重摆
    void submit();                 // 交牌(含AI组牌) -> 比牌
    void autoSubmit();             // 超时自动摆完并交牌
    bool allPlaced() const;        // 9 张是否全部摆完

    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    TextBox hint_;
    std::array<CardSprite, 9> handSprites_;            // 手牌区
    std::array<std::array<CardSprite, 3>, 3> lineSprites_;  // 三道已摆牌
    std::array<sf::RectangleShape, 9> handSlotRects_;       // 手牌空位框
    std::array<std::array<sf::RectangleShape, 3>, 3> lineSlotRects_;  // 槽位框
    std::array<Button, 3> lineBtns_;   // 头/中/尾道选择
    Button btnReset_;                  // 重置
    Button btnSubmit_;                 // 交牌
    CountdownBar countdown_;           // 20 秒倒计时
    std::array<bool, 9> handUsed_{};   // 手牌是否已用
    std::array<std::array<bool, 3>, 3> slotUsed_{};  // 槽是否已占
    int currentLine_ = 0;              // 当前选中的道(0头/1中/2尾)
    bool submitted_ = false;           // 已交牌(防重复)
    bool timeoutFired_ = false;        // 超时自动交牌只触发一次(场景实例级,非static)
};
