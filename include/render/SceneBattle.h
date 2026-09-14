#pragma once
#include "render/SceneManager.h"
#include "render/CardSprite.h"
#include "ui/TextBox.h"
#include "ui/Avatar.h"
#include <array>

// ====== 比牌界面场景(阶段6) ======
// 只显示"当前比牌道"的牌面(不一次性放 9 张),支持 2~6 人:
//   1=你(下方大牌) 2=右中下 3=右中上 4=上方靠右 5=上方靠左 6=左中(座位表 SEATS)
//   人数 -> 座位号: 2人{1,4} 3人{1,4,5} 4人{1,2,3,6} 5人{1,2,3,5,6} 6人{1..6}
//   所有座位(牌组顶 + 头像顶)均不高于标题下边沿(TITLE_BOTTOM, 编译期 static_assert 护栏)
//   每位玩家统一使用"头像+昵称"系统(seatAvatars_, 替换原纯昵称标签)
// 流程:每道 0.8s 牌背 -> 翻正 + 中央显示该道赢家/牌型,
//       停留 2.5s 自动切下一道(替换牌面),三道比完 3 秒自动进结算。
// 牌型/赢家由 HandEvaluator + Round::findWinners 计算(纯展示,不结算筹码)。

class SceneBattle : public Scene {
public:
    explicit SceneBattle(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    void loadLine(int lineId, bool faceUp);  // 装载某道所有玩家的牌(或牌背)
    void flipUp();                           // 当前道翻正 + 显示结果
    void advance();                          // 进入下一道(或完成)

    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;        // "第 X 局 比牌"
    TextBox info_;         // 中央结果文字(金色大字)
    TextBox lineTag_;      // 当前道 "头道 1/3"
    std::array<std::array<CardSprite, 3>, 6> cards_;   // [玩家0..5][位置] 当前道 3 张
    std::array<Avatar, 6> seatAvatars_;                // 各玩家头像+昵称名牌(与牌组同座)
    int playerCount_ = 0;
    int showLine_ = 0;     // 当前比牌道 0..2
    int phase_ = 0;        // 0=等翻正 1=展示结果 2=比完等3秒自动进结算
    float timer_ = 0.f;
    bool showNext_ = false;    // 三道比完,3 秒后自动进结算
};
