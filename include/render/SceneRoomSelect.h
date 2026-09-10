#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include "ui/ChipBar.h"
#include "ui/Avatar.h"
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
    void onHomePressed() override;   // home 键 = 返回大厅

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
    ChipBar chipBar_;
    Avatar  selfAvatar_;              // 局外: 本人头像(筹码条左侧)

    // 入场资格提示弹窗(余额 < 该房间第一局注金 ante 时点击房间弹出)
    bool notEnough_ = false;
    sf::RectangleShape overlay_;
    sf::RectangleShape dialog_;
    TextBox denyTitle_;
    TextBox denyText_;
    Button btnDenyOk_;

    // ---- 难度选择(成员B 接入: 简单/中等/困难) ----
    bool diffOpen_ = false;
    sf::RectangleShape diffDialog_;
    TextBox diffTitle_;
    TextBox diffDesc_;
    std::array<Button, 3> diffBtns_;
    Button btnDiffOpen_;
    Button btnDiffClose_;
    void openDiffPopup();
    void applyDifficulty(int idx);
    void refreshDiffColors();
};
