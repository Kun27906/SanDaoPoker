#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include "ui/ChipBar.h"
#include <array>

// ====== 结算界面(成员C) ======
// 非最终局: 显示每家"目前筹码 (本局盈亏)", 盈利鲜绿/亏损鲜红, 居中;
//           按钮 [下一局] + [逃跑](罚 100 提前结束本场)。
// 最终局(打完全部轮次 或 逃跑): 只显示玩家个人筹码与每局盈亏明细+总盈亏,
//           无"下一局", 下方居中 [返回大厅]。
// 踢出机制: 每局结算后检测真人牌桌筹码是否够付下一局底注(3×底注),
//           不够 -> 弹窗告知被踢出本场(不扣逃跑费), 确定后回大厅;
//           破产补充(余额<100)统一在大厅检测弹出。

class SceneResult : public Scene {
public:
    explicit SceneResult(SceneManager* mgr);
    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    void settleAndSync();          // 结算本局 + 账号同步 + 踢出判定
    void refreshRows();            // 刷新各家筹码/盈亏行
    void rebuildFinalText();       // 组装最终结算明细文字
    void escape();                 // 逃跑: 罚100 -> 立即最终结算
    void confirmKickOut();         // 踢出弹窗确定 -> 回大厅
    void nextRound();

    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    std::array<TextBox, MAX_PLAYERS> playerRows_;  // 非最终局: 各家行
    TextBox detailText_;   // 最终结算明细(每局盈亏+总盈亏)
    TextBox tipText_;      // 提示文字
    Button btnNext_;       // 下一局
    Button btnEscape_;     // 逃跑
    Button btnLobby_;      // 返回大厅(最终结算用, 居中)
    ChipBar chipBar_;

    // 踢出弹窗
    bool kickPending_ = false;
    sf::RectangleShape overlay_;
    sf::RectangleShape dialog_;
    TextBox dialogText_;
    TextBox dialogSub_;
    Button btnDialogOk_;

    bool final_ = false;        // 最终结算模式(打完或逃跑)
    int escapePenalty_ = 0;     // 逃跑罚金(显示在明细)
    bool synced_ = false;       // 本局只结算同步一次
};
