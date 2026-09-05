#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include "ui/ChipBar.h"
#include <array>

// ====== 第二界面: 真大厅(成员C) ======
// 五个大按钮按人数(2/3/4/5/6)进入对应房间列表; 右上角实时显示账号筹码。
class SceneLobby : public Scene {
public:
    explicit SceneLobby(SceneManager* mgr);
    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    TextBox hint_;
    std::array<Button, 5> btnSeats_;  // 2~6人
    ChipBar chipBar_;
};
