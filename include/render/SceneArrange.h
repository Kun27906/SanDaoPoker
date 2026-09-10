#pragma once
#include "render/SceneManager.h"
#include "render/CardSprite.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include "ui/CountdownBar.h"
#include "ui/ChipBar.h"
#include <array>

// ====== 组牌界面(阶段5: 分三道) ======
// 玩法:9 张手牌分配到三道(头/中/尾,每道3张=共9个预定槽位)。
//  - 拖拽: 按住一张牌拖动到任意预定槽位, 靠近槽位时自动吸附;
//  - 放回: 单击已放置的牌 -> 牌"飞回"下方原手牌位(无需一键重置);
//  - 音效: 放入槽位 / 放回手牌 均插入点击音效;
//  - 点击手牌仍可直接放入当前选中道(左侧选道, 兼容旧操作);
//  - 限时 40 秒, 归零自动摆完并交牌。
// 交牌后:AI 玩家按难度自动组牌, 切到比牌界面。

class SceneArrange : public Scene {
public:
    explicit SceneArrange(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    void placeCard(int handIdx);           // 点击手牌 -> 放入当前道空槽
    void placeAt(int handIdx, int line, int pos);  // 把某张手牌放到指定槽
    void returnToHand(int line, int pos);  // 单击已放置牌 -> 飞回手牌
    void resetArrange();                   // 清空重摆
    void submit();                         // 交牌(含AI组牌) -> 比牌
    void autoSubmit();                     // 超时自动摆完并交牌
    bool allPlaced() const;                // 9 张是否全部摆完
    void refreshSlotSprites();             // 依据 slotHand_ 刷新三道牌面
    void rebuildLines();                   // slotHand_ -> 房间真人的 lines
    sf::Vector2f slotPos(int line, int pos) const;
    int  nearestSlot(const sf::Vector2f& cardTopLeft, float maxDist) const;  // -> line*3+pos, -1 无
    void beginDrag(int handIdx, int fromLine, int fromPos, const sf::Vector2f& mouse);
    void dropDrag();                       // 松开鼠标: 落位/吸附/交换/回位
    void startFlyBack(int handIdx);        // 启动"飞回手牌"动画

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
    CountdownBar countdown_;           // 40 秒倒计时
    std::array<bool, 9> handUsed_{};   // 手牌是否已放置
    std::array<std::array<int, 3>, 3> slotHand_;     // 每槽的手牌下标(-1 空)
    int currentLine_ = 0;              // 当前选中的道(0头/1中/2尾)
    bool submitted_ = false;           // 已交牌(防重复)
    bool timeoutFired_ = false;        // 超时自动交牌只触发一次
    ChipBar chipBar_;                  // 右上角账号筹码

    // ---- 拖拽状态 ----
    CardSprite dragSprite_;            // 拖动中的牌
    bool pendingDrag_ = false;         // 已按下某张牌(尚未判定拖拽/点击)
    bool dragging_ = false;            // 正在拖动
    int  dragHand_ = -1;               // 被拖动的手牌下标
    int  dragFromLine_ = -1;           // 拖拽来源槽(来自手牌时为 -1)
    int  dragFromPos_ = -1;
    sf::Vector2f dragPos_{0.f, 0.f};   // 拖动牌左上角当前位置
    sf::Vector2f dragGrab_{0.f, 0.f};  // 鼠标相对牌左上角的抓取偏移
    sf::Vector2f pressPos_{0.f, 0.f};  // 按下点(判定点击/拖拽)
    int  snapSlot_ = -1;               // 当前吸附到的高亮槽(-1 无)

    // ---- 飞回动画 ----
    CardSprite flySprite_;
    bool flying_ = false;
    int  flyHand_ = -1;                // 飞回的手牌下标
    sf::Vector2f flyFrom_{0.f, 0.f};
    sf::Vector2f flyTo_{0.f, 0.f};
    float flyT_ = 0.f;
};
