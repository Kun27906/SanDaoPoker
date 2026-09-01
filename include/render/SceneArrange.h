#pragma once
#include "render/SceneManager.h"
#include "render/CardSprite.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include "ui/CountdownBar.h"
#include <array>

// ====== 组牌界面场景(阶段3骨架) ======
// 阶段5将实现完整组牌(9张手牌分三道+交牌)。
// 当前:显示 3 张随机牌(验证 CardSprite+素材)+ 1 张牌背 + 倒计时条演示。

class SceneArrange : public Scene {
public:
    explicit SceneArrange(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    TextBox hint_;
    std::array<CardSprite, 3> cardSprites_;  // 3 张随机牌(正面)
    CardSprite backSprite_;                  // 1 张牌背
    CountdownBar countdown_;                 // 倒计时条演示
    Button btnBack_;                         // 返回菜单
};
