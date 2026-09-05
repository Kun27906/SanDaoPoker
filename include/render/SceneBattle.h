#pragma once
#include "render/SceneManager.h"
#include "render/CardSprite.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include <array>

// ====== 比牌界面场景(阶段6,布局重构版) ======
// 只显示"当前比牌道"的牌面(不一次性放 9 张):
//   真人(下) 3 张 scale 0.5 = 与组牌界面一致大小;
//   AI(上/左/右) 3 张 scale 0.3 较小,保证画面完整放得下。
// 流程:每道 0.8s 牌背 -> 翻正 + 中央显示该道赢家/牌型,
//       停留 2.5s 自动切下一道(替换牌面),三道比完出现"查看结算"。
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
    std::array<std::array<CardSprite, 3>, 4> cards_;   // [玩家][位置] 当前道 3 张
    std::array<TextBox, 4> nameTags_;                  // 玩家名
    Button btnNext_;       // 查看结算
    int playerCount_ = 0;
    int showLine_ = 0;     // 当前比牌道 0..2
    int phase_ = 0;        // 0=等翻正 1=展示结果
    float timer_ = 0.f;
    bool showNext_ = false;    // 三道比完,显示按钮
    bool revealed_ = false;    // 当前道已翻正
};
