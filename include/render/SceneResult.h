#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"

// ====== 结算界面场景(阶段7) ======
// 进入时结算本局: Room::settleRound()(逐道结算筹码,一次性)
// 显示: 左侧本局结果(每道赢家/牌型/筹码变化,多行),
//       右侧当前总账排名(Room::getRanking),
//       比赛打完显示最终排名。
// 按钮: [下一局] 开新局回组牌; [返回主菜单] 回选房。

class SceneResult : public Scene {
public:
    explicit SceneResult(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    void doSettle();   // 结算本局(只调用一次)
    void nextRound();  // 开始下一局

    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;       // "第X局结果"
    TextBox resultText_;  // 本局结算文字(多行,左列)
    TextBox rankText_;    // 总账排名(右列)
    TextBox finalText_;   // 全部轮次打完的最终提示
    Button btnNext_;      // 下一局
    Button btnMenu_;      // 返回主菜单
    bool settled_ = false;
    bool finished_ = false;  // 比赛是否全部打完
};
