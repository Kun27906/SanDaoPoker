#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include "ui/ChipBar.h"
#include <array>

// ====== 第三界面: 选房间(成员C, 原 SceneMenu 重构) ======
// 从大厅按人数进入后, 只显示该人数对应的房间(ROOM_CONFIGS 过滤),
// 按钮居中; 点[开始游戏]创建 Room(真人+AI 入场筹码=账号余额)进入组牌。
class SceneRoomSelect : public Scene {
public:
    explicit SceneRoomSelect(SceneManager* mgr);
    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    void refreshColors();
    void startGame();

    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    TextBox hint_;
    std::array<Button, 6> roomBtns_;  // 同人数最多 5 个房间(+1 冗余)
    int roomIndex_[6] = {-1, -1, -1, -1, -1, -1};  // 全局配置下标
    int roomCount_ = 0;
    int selected_ = 0;
    Button btnStart_;
    Button btnBack_;
    ChipBar chipBar_;
};
