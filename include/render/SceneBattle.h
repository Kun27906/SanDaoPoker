#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"

// ====== 比牌界面场景(阶段3骨架) ======
// 阶段6将实现四家牌面+逐道翻牌动画+胜负标记。
// 当前:标题占位 + 场景切换演示。

class SceneBattle : public Scene {
public:
    explicit SceneBattle(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    TextBox hint_;
    Button btnNext_;   // 进入结算
    Button btnBack_;   // 返回菜单
};
