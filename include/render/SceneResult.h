#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"

// ====== 结算界面场景(阶段3骨架) ======
// 阶段7将实现筹码变化(Round::settle)+总账排名(Room::getRanking)。
// 当前:标题占位 + 返回菜单。

class SceneResult : public Scene {
public:
    explicit SceneResult(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    TextBox hint_;
    Button btnBack_;   // 返回主菜单
};
