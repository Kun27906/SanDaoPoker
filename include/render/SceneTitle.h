#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"

// ====== 第一界面: 启动页(成员C) ======
// 只显示标题 + [开始游戏]; 后续可在此扩展设置/调试入口。
// 点击开始游戏 -> 进入第二界面(大厅 SceneLobby)

class SceneTitle : public Scene {
public:
    explicit SceneTitle(SceneManager* mgr);
    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    TextBox subtitle_;
    TextBox version_;
    Button btnStart_;
};
