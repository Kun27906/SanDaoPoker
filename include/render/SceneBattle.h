#pragma once
#include "render/SceneManager.h"
#include "render/CardSprite.h"
#include "ui/TextBox.h"
#include "ui/Avatar.h"
#include <array>

// 只显示"当前比牌道"的牌面,支持 2~6 人:
// 1=你 2=右中下 3=右中上 4=上方靠右 5=上方靠左 6=左中
// 人数 -> 座位号: 2人{1,4} 3人{1,4,5} 4人{1,2,3,6} 5人{1,2,3,5,6} 6人{1..6}
// 所有座位均不高于标题下边沿
// 每位玩家统一使用"头像+昵称"系统
// 流程:每道 0.8s 牌背 -> 翻正 + 中央显示该道赢家/牌型,
// 停留 2.5s 自动切下一道,三道比完 3 秒自动进结算。
// 牌型/赢家由 HandEvaluator + Round::findWinners 计算。

class SceneBattle : public Scene {
public:
    explicit SceneBattle(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    void loadLine(int lineId, bool faceUp);  // 装载某道所有玩家的牌
    void flipPlayer(int p);                  // 翻开某一位玩家的当前道 3 张
    void revealWinner();                     // 最后一家翻完停留结束后: 计算并显示本道赢家/牌型
    void advance();                          // 进入下一道

    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;        // "第 X 局 比牌"
    TextBox info_;         // 中央结果文字
    TextBox lineTag_;      // 当前道 "头道 1/3"
    std::array<std::array<CardSprite, 3>, 6> cards_;   // [玩家0..5][位置] 当前道 3 张
    std::array<Avatar, 6> seatAvatars_;                // 各玩家头像+昵称名牌
    int playerCount_ = 0;
    int showLine_ = 0;     // 当前比牌道 0..2
    int phase_ = 0;
    int flipIndex_ = -1;   // 逐家翻牌进度: 已翻到的玩家下标
    float timer_ = 0.f;
    bool showNext_ = false;    // 三道比完,3 秒后自动进结算
};
