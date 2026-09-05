#pragma once
#include "render/SceneManager.h"
#include "render/CardSprite.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include <array>

// ====== 比牌界面场景(阶段6) ======
// 四家牌面(上=AI-1 左=AI-2 右=AI-3 下=你)展示,
// 三道逐组翻牌动画:进入后每 1 秒揭示一道(头->中->尾),
// 揭示时该道所有玩家牌同时翻正,中央信息显示该道赢家与牌型
// (HandEvaluator::evaluate + Round::findWinners,纯展示不结算筹码)。
// 三道揭示完出现"查看结算"按钮,进入结算界面(筹码在结算界面统一结算)。

class SceneBattle : public Scene {
public:
    explicit SceneBattle(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    void revealNext();   // 揭示下一道(并计算赢家/牌型信息)
    bool allRevealed() const { return revealLine_ >= 3; }

    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    TextBox info_;       // 中央信息:当前道比牌结果
    std::array<std::array<std::array<CardSprite, 3>, 3>, 4> cardSprites_;  // [玩家][道][位置]
    std::array<TextBox, 4> nameTags_;    // 玩家名(上方/左/右/下)
    std::array<TextBox, 4> chipTags_;    // 玩家筹码
    std::array<bool, 3> lineRevealed_{false, false, false};  // 各道是否已揭示
    int revealLine_ = -1;    // 当前揭示进度:-1 未开始, 0..2 已揭示到某道, >=3 完成
    float animTimer_ = 0.f;  // 翻牌动画计时
    Button btnNext_;         // 查看结算(三道揭示完后可用)
    bool showNext_ = false;
};
