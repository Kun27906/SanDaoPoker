#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"

// ====== 主菜单场景(阶段3骨架) ======
// 阶段4将在此实现房间选择(ROOM_CONFIGS 16 种)。
// 当前:显示标题 + 三个入口按钮,验证状态机切换。

class SceneMenu : public Scene {
public:
    explicit SceneMenu(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    SceneManager* mgr_;
    sf::Sprite bg_;              // 桌面背景
    TextBox title_;              // 游戏标题
    TextBox subtitle_;           // 副标题
    Button btnArrange_;          // 进入组牌界面
    Button btnBattle_;           // 进入比牌界面
    Button btnResult_;           // 进入结算界面
};
