#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include <array>

// ====== 主菜单场景(阶段4:选房间) ======
// 显示 16 种房间(ROOM_CONFIGS: 人数/底注/轮次),点击选中(高亮),
// 点"开始游戏"创建 Room 并进入组牌界面。

class SceneMenu : public Scene {
public:
    explicit SceneMenu(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    void refreshRoomButtonColors();  // 刷新选中高亮
    void startGame();                // 创建 Room 并进入组牌

    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    TextBox subtitle_;
    std::array<Button, 22> roomBtns_;  // 22 个房间按钮(两列)
    Button btnStart_;                  // 开始游戏
    int selected_ = 7;                 // 默认选中"4人·注500·6局"(下标7)
};
