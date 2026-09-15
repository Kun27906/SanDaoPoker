#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include "ui/ChipBar.h"
#include "ui/Avatar.h"
#include <array>

// 非最终局: 每家"目前筹码 "独立着色, 整块居中;
// 块外加弹窗同色边框+底色面板突出; 按钮 [下一局] + [逃跑]。
// 最终局: 每局盈亏逐行独立着色 + 总盈亏 + 当前筹码,
// 每局结算块 与 总盈亏结算块 各自带边框+底色面板, 整块居中; [返回大厅]。
// 提示行统一亮红色。
// 踢出机制: 每局结算后筹码不足下一局注金 -> 弹窗告知被踢出。
// 胜负音效: 仅在"整场结束"时按整场总盈亏播放一次, 单局不播。

class SceneResult : public Scene {
public:
    explicit SceneResult(SceneManager* mgr);
    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;
    void onHomePressed() override;   // home: 非最终局=逃跑, 最终局=返回大厅

private:
    void settleAndSync();          // 结算本局 + 账号同步 + 踢出判定
    void refreshRows();            // 刷新各家筹码/盈亏行
    void rebuildFinalText();       // 组装最终结算明细
    void layoutRows();             // 非最终局: 居中排布面板/提示/按钮
    void layoutFinal();            // 最终局: 居中排布两个面板/提示/按钮
    void escape();                 // 逃跑: 梯度罚金 -> 立即最终结算
    void confirmKickOut();         // 踢出弹窗确定 -> 回大厅
    void nextRound();
    int  matchTotal() const;       // 整场总盈亏
    void playFinalSound();         // 整场结束音效
    void startCoinsPhase();        // 点击[返回大厅]/home: 播 coins 音 + 盈亏数字跳动
    void buildKickDialog();        // 组装踢出弹窗文案
    void reevaluateKick();         // 局内筹码被外部改动 -> 重判踢出

    static constexpr int MAX_ROUND_LINES = 18;   // 最多 16 局 + 逃跑罚 行

    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    std::array<TextBox, MAX_PLAYERS> playerRows_;  // 非最终局: 各家行

    // 最终局: 每局盈亏逐行
    std::array<TextBox, MAX_ROUND_LINES> roundLines_;
    int  roundLineCount_ = 0;
    TextBox totalLine_;     // 总盈亏
    TextBox chipsLine_;     // 当前筹码

    // 高亮面板
    sf::RectangleShape panelRows_;    // 非最终局: 各家筹码块
    sf::RectangleShape panelRounds_;  // 最终局: 单一面板

    Button btnNext_;       // 下一局
    Button btnEscape_;     // 逃跑
    Button btnLobby_;      // 返回大厅
    ChipBar chipBar_;
    std::array<Avatar, MAX_PLAYERS> avatars_;  // 局内头像: [0]本人左下, [1..]他人右侧居中

    // 踢出弹窗
    bool kickPending_ = false;
    sf::RectangleShape overlay_;
    sf::RectangleShape dialog_;
    TextBox dialogText_;
    TextBox dialogSub_;
    Button btnDialogOk_;

    bool final_ = false;        // 最终结算模式
    int escapePenalty_ = 0;     // 逃跑罚金
    int lastChips_ = 0;         // 上次踢出判定用的本人局内筹码
    bool synced_ = false;       // 本局只结算同步一次
    bool settleSoundDone_ = false;  // 胜负音是否播完
    bool coinsStarted_ = false;     // coins 音效+盈亏动画已启动
    bool betting_ = false;
};
