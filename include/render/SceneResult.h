#pragma once
#include "render/SceneManager.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include "ui/ChipBar.h"
#include "ui/Avatar.h"
#include <array>

// 单局结算与非最终局同屏; 整场结束后改为明细面板
class SceneResult : public Scene {
public:
    explicit SceneResult(SceneManager* mgr);
    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;
    void onHomePressed() override;   // 非最终局=逃跑, 最终局=返回大厅

private:
    void settleAndSync();
    void refreshRows();
    void rebuildFinalText();
    void layoutRows();
    void layoutFinal();
    void escape();
    void confirmKickOut();
    void nextRound();
    int  matchTotal() const;
    void playFinalSound();
    void startCoinsPhase();
    void buildKickDialog();
    void reevaluateKick();

    static constexpr int MAX_ROUND_LINES = 18;   // 16 局 + 逃跑罚

    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    std::array<TextBox, MAX_PLAYERS> playerRows_;

    std::array<TextBox, MAX_ROUND_LINES> roundLines_;
    int  roundLineCount_ = 0;
    TextBox totalLine_;
    TextBox chipsLine_;

    sf::RectangleShape panelRows_;
    sf::RectangleShape panelRounds_;

    Button btnNext_;
    Button btnEscape_;
    Button btnLobby_;
    ChipBar chipBar_;
    std::array<Avatar, MAX_PLAYERS> avatars_;

    bool kickPending_ = false;
    sf::RectangleShape overlay_;
    sf::RectangleShape dialog_;
    TextBox dialogText_;
    TextBox dialogSub_;
    Button btnDialogOk_;

    bool final_ = false;
    int escapePenalty_ = 0;
    int lastChips_ = 0;
    bool synced_ = false;
    bool settleSoundDone_ = false;
    bool coinsStarted_ = false;
    bool betting_ = false;
};
