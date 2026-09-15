#include "render/SceneResult.h"
#include "render/AssetManager.h"
#include "render/SoundManager.h"
#include "render/Account.h"
#include "core/Room.h"
#include "ui/PanelFrame.h"
#include <cstdio>

namespace {
constexpr unsigned WW = layout::WINDOW_W;
constexpr unsigned WH = layout::WINDOW_H;
const sf::Color C_WIN(0, 200, 90);
const sf::Color C_LOSE(255, 70, 70);
const sf::Color C_GOLD(255, 215, 0);
const sf::Color C_ALERT(255, 80, 80);
const sf::Color C_PANEL_BG(30, 40, 70);
constexpr float PANEL_W = 620.f;
constexpr float PANEL_PAD = 24.f;
}

SceneResult::SceneResult(SceneManager* mgr) : mgr_(mgr) {
    scene_setup::background(bg_, AssetManager::instance().background(), WW, WH);

    title_.setCharacterSize(36);
    title_.setColor(C_GOLD);
    title_.centerOrigin();

    for (int i = 0; i < MAX_PLAYERS; i++) {
        playerRows_[i].setCharacterSize(26);
        playerRows_[i].centerOrigin();
    }
    for (int i = 0; i < MAX_ROUND_LINES; i++) {
        roundLines_[i].setCharacterSize(24);
        roundLines_[i].centerOrigin();
    }
    totalLine_.setCharacterSize(28);
    totalLine_.centerOrigin();
    chipsLine_.setCharacterSize(24);
    chipsLine_.setColor(C_GOLD);
    chipsLine_.centerOrigin();

    for (sf::RectangleShape* p : {&panelRows_, &panelRounds_}) {
        p->setFillColor(C_PANEL_BG);
        p->setOutlineColor(C_GOLD);
        p->setOutlineThickness(3.f);
    }

    btnNext_.setText("下一局");
    btnNext_.setSize(sf::Vector2f(300.f, 60.f));
    btnNext_.setCallback([this]() { nextRound(); });

    btnEscape_.setText("逃跑");
    btnEscape_.setSize(sf::Vector2f(300.f, 60.f));
    btnEscape_.setCallback([this]() { escape(); });

    btnLobby_.setText("返回大厅");
    btnLobby_.setSize(sf::Vector2f(420.f, 60.f));
    btnLobby_.setCallback([this]() { startCoinsPhase(); });

    scene_setup::chipBar(chipBar_, WW);

    {
        int pc = mgr_->room->playerCount;
        int na = pc - 1;
        if (na < 1) na = 1;
        const float aiX = 1185.f, aiCY = 400.f, aiGap = 80.f, aiR = 22.f;
        float totalH = (na - 1) * aiGap;
        for (int i = 0; i < pc && i < MAX_PLAYERS; i++) {
            avatars_[i].setNickname(mgr_->room->players[i].name);
            avatars_[i].setTexture(AssetManager::instance().avatarTexture(i));
            if (i == 0) {
                avatars_[i].setRadius(28.f);
                avatars_[i].setMinPlateWidth(150.f);
                avatars_[i].setSelfStyle(true);
                avatars_[i].setCenter(sf::Vector2f(78.f, 700.f));
            } else {
                avatars_[i].setRadius(aiR);
                avatars_[i].setCenter(sf::Vector2f(
                    aiX, aiCY - totalH / 2.f + (i - 1) * aiGap));
            }
        }
    }

    overlay_.setSize(sf::Vector2f(WW, WH));
    overlay_.setFillColor(sf::Color(0, 0, 0, 160));
    dialog_.setSize(sf::Vector2f(680.f, 250.f));
    dialog_.setPosition(sf::Vector2f((WW - 680.f) / 2.f, (WH - 250.f) / 2.f));
    dialog_.setFillColor(C_PANEL_BG);
    dialog_.setOutlineColor(C_GOLD);
    dialog_.setOutlineThickness(3.f);
    dialogText_.setCharacterSize(24);
    dialogText_.setColor(sf::Color::White);
    dialogText_.centerOrigin();
    dialogText_.setPosition(sf::Vector2f(WW / 2.f, (WH - 250.f) / 2.f + 78.f));
    dialogSub_.setCharacterSize(18);
    dialogSub_.setColor(sf::Color(230, 200, 150));
    dialogSub_.centerOrigin();
    dialogSub_.setPosition(sf::Vector2f(WW / 2.f, (WH - 250.f) / 2.f + 128.f));
    btnDialogOk_.setText("确定");
    btnDialogOk_.setPosition(sf::Vector2f(WW / 2.f - 100.f, (WH - 250.f) / 2.f + 172.f));
    btnDialogOk_.setSize(sf::Vector2f(200.f, 52.f));
    btnDialogOk_.setCallback([this]() { confirmKickOut(); });

    settleAndSync();
    // 踢出视同提前结束, 否则 final_ 被覆盖成 false, 两个布局都不执行
    final_ = mgr_->room->isFinished() || kickPending_;
    if (!final_ && !kickPending_) refreshRows();
    if (final_) {
        // 整场结束: 未点返回大厅前不显示结算后筹码
        chipBar_.setImmediate(Account::instance().balance() - matchTotal());
        rebuildFinalText();
        playFinalSound();
    } else {
        Room* room = mgr_->room.get();
        int ante = room->config.ante;
        int d0 = room->historyCount > 0 ? room->roundHistory[room->historyCount - 1][0] : 0;
        int after = Account::instance().balance();
        chipBar_.setImmediate(after - d0 - ante);
        chipBar_.rollTo(after, 0.8f);
        SoundManager::instance().playCoins();
    }
}

// 整场总盈亏
int SceneResult::matchTotal() const {
    Room* room = mgr_->room.get();
    int t = -escapePenalty_;
    for (int r = 0; r < room->historyCount; r++) t += room->roundHistory[r][0];
    return t;
}

// 总盈亏定胜负音
void SceneResult::playFinalSound() {
    int t = matchTotal();
    if (t > 0)      SoundManager::instance().playWin();
    else if (t < 0) SoundManager::instance().playLose();
}

// 金币音 + 筹码盈亏跳动, 播完自动回大厅
void SceneResult::startCoinsPhase() {
    if (coinsStarted_) return;
    coinsStarted_ = true;
    SoundManager::instance().playCoins();
    int t = matchTotal();
    int after = Account::instance().balance();
    if (chipBar_.isRolling()) {
        chipBar_.rollTo(after, 1.0f);
    } else {
        chipBar_.setImmediate(after - t);
        chipBar_.rollTo(after, 1.0f);
    }
}

// 结算并同步账号, 同时判定踢出
void SceneResult::settleAndSync() {
    if (synced_) return;
    synced_ = true;
    Room* room = mgr_->room.get();
    room->settleRound();

    if (room->historyCount > 0) {
        int d0 = room->roundHistory[room->historyCount - 1][0];
        Account::instance().add(d0);
    }

    // 结算后筹码不足下一局底注则踢出; 开发者模式补足后由 reevaluateKick 撤销
    if (!room->isFinished() && room->players[0].chips < room->config.ante) {
        kickPending_ = true;
        final_ = true;
        buildKickDialog();
    }
    lastChips_ = room->players[0].chips;
}

void SceneResult::buildKickDialog() {
    Room* room = mgr_->room.get();
    char t[128];
    std::snprintf(t, sizeof(t), "您的筹码 (%d) 不足以支付下一局底注 (%d)，\n您已被踢出本场对局",
                  room->players[0].chips, room->config.ante);
    dialogText_.setText(t);
    dialogSub_.setText("本次不扣除逃跑费用 · 点击确定返回大厅");
}

// 局内筹码被外部改动后重判踢出
void SceneResult::reevaluateKick() {
    Room* room = mgr_->room.get();
    if (!room || !synced_ || room->isFinished() || betting_) return;
    const bool tooPoor = room->players[0].chips < room->config.ante;
    if (tooPoor && !kickPending_) {
        kickPending_ = true;
        final_ = true;
        buildKickDialog();
        rebuildFinalText();
    } else if (!tooPoor && kickPending_) {
        kickPending_ = false;
        final_ = false;
        refreshRows();
    }
}

void SceneResult::refreshRows() {
    Room* room = mgr_->room.get();
    char t[48];
    std::snprintf(t, sizeof(t), "第 %d 局 结算", room->currentRound);
    title_.setText(t);

    if (!final_) {
        int last = room->historyCount - 1;
        for (int i = 0; i < room->playerCount; i++) {
            int d = (last >= 0) ? room->roundHistory[last][i] : 0;
            int chips = room->players[i].chips;
            const char* dn = (i == 0) ? "你" : room->players[i].name.c_str();
            char buf[96];
            std::snprintf(buf, sizeof(buf), "%s      %d 筹码  (%+d)", dn, chips, d);
            playerRows_[i].setText(buf);
            playerRows_[i].setColor(d > 0 ? C_WIN : (d < 0 ? C_LOSE : sf::Color(235, 235, 235)));
        }
        layoutRows();
    } else {
        rebuildFinalText();
    }
}

void SceneResult::rebuildFinalText() {
    Room* room = mgr_->room.get();
    int total = 0;
    int n = 0;
    for (int r = 0; r < room->historyCount && n < MAX_ROUND_LINES; r++) {
        int d = room->roundHistory[r][0];
        total += d;
        char line[96];
        std::snprintf(line, sizeof(line), "第 %d 局:  %+d", r + 1, d);
        roundLines_[n].setText(line);
        roundLines_[n].setColor(d > 0 ? C_WIN : (d < 0 ? C_LOSE : sf::Color(235, 235, 235)));
        n++;
    }
    if (escapePenalty_ != 0 && n < MAX_ROUND_LINES) {
        total -= escapePenalty_;
        char line[96];
        std::snprintf(line, sizeof(line), "逃跑罚:  -%d", escapePenalty_);
        roundLines_[n].setText(line);
        roundLines_[n].setColor(C_LOSE);
        n++;
    }
    roundLineCount_ = n;

    char buf[96];
    std::snprintf(buf, sizeof(buf), "总盈亏:  %+d", total);
    totalLine_.setText(buf);
    totalLine_.setColor(total >= 0 ? C_WIN : C_LOSE);
    std::snprintf(buf, sizeof(buf), "当前筹码:  %d", Account::instance().balance());
    chipsLine_.setText(buf);

    title_.setText(escapePenalty_ ? "本场提前结束" : "本场结束");

    layoutFinal();
}

// 非最终局居中排布
void SceneResult::layoutRows() {
    const float cx = WW / 2.f;
    int pc = mgr_->room->playerCount;
    if (pc < 1) pc = 1;

    const float titleH = 44.f, tGap = 20.f, rowH = 44.f;
    const float tipGap = 28.f, tipH = 26.f, bGap = 22.f, btnH = 60.f;
    float rowsH = PANEL_PAD * 2 + pc * rowH;
    float assemblyH = titleH + tGap + rowsH + tipGap + tipH + bGap + btnH;
    float top = (WH - assemblyH) / 2.f;
    if (top < 12.f) top = 12.f;

    title_.setPosition(sf::Vector2f(cx, top + titleH / 2.f));
    float blockTop = top + titleH + tGap;

    panelRows_.setSize(sf::Vector2f(PANEL_W, rowsH));
    panelRows_.setPosition(sf::Vector2f(cx - PANEL_W / 2.f, blockTop));
    for (int i = 0; i < pc; i++) {
        playerRows_[i].setPosition(sf::Vector2f(cx, blockTop + PANEL_PAD + i * rowH + rowH / 2.f));
    }

    float btnY = blockTop + rowsH + tipGap + tipH + bGap;
    const float bw = 300.f, bg = 40.f;
    float left = cx - (bw * 2.f + bg) / 2.f;
    btnNext_.setPosition(sf::Vector2f(left, btnY));
    btnNext_.setSize(sf::Vector2f(bw, btnH));
    btnEscape_.setPosition(sf::Vector2f(left + bw + bg, btnY));
    btnEscape_.setSize(sf::Vector2f(bw, btnH));
}

// 最终局居中排布, 面板随明细行数增长
void SceneResult::layoutFinal() {
    const float cx = WW / 2.f;
    float lineH = (roundLineCount_ > 8) ? 24.f : 32.f;
    unsigned fs = (roundLineCount_ > 8) ? 20u : 24u;
    for (int i = 0; i < roundLineCount_; i++) {
        roundLines_[i].setCharacterSize(fs);
    }
    totalLine_.setCharacterSize(fs + 2);
    chipsLine_.setCharacterSize(fs);

    const float sectionGap = 16.f;
    float roundsH = roundLineCount_ * lineH;
    float contentH = roundsH + sectionGap + 2 * lineH;
    float panelH = PANEL_PAD * 2 + contentH;

    const float titleH = 44.f, tGap = 20.f, tipGap = 28.f, tipH = 26.f, bGap = 22.f, btnH = 60.f;
    float assemblyH = titleH + tGap + panelH + tipGap + tipH + bGap + btnH;
    float top = (WH - assemblyH) / 2.f;
    if (top < 12.f) top = 12.f;

    title_.setPosition(sf::Vector2f(cx, top + titleH / 2.f));
    float panelTop = top + titleH + tGap;

    panelRounds_.setSize(sf::Vector2f(PANEL_W, panelH));
    panelRounds_.setPosition(sf::Vector2f(cx - PANEL_W / 2.f, panelTop));
    float y = panelTop + PANEL_PAD + lineH / 2.f;
    for (int i = 0; i < roundLineCount_; i++) {
        roundLines_[i].setPosition(sf::Vector2f(cx, y));
        y += lineH;
    }
    y += sectionGap;
    totalLine_.setPosition(sf::Vector2f(cx, y));
    y += lineH;
    chipsLine_.setPosition(sf::Vector2f(cx, y));

    float btnY = panelTop + panelH + tipGap + tipH + bGap;
    btnLobby_.setPosition(sf::Vector2f(cx - 210.f, btnY));
    btnLobby_.setSize(sf::Vector2f(420.f, btnH));
}

void SceneResult::confirmKickOut() {
    kickPending_ = false;
    mgr_->changeTo(SceneId::Lobby);
}

void SceneResult::onHomePressed() {
    if (kickPending_) return;
    if (final_) {
        if (!settleSoundDone_ || coinsStarted_) return;
        startCoinsPhase();
    } else {
        escape();
    }
}

void SceneResult::escape() {
    Room* room = mgr_->room.get();
    if (final_ || !room) return;
    final_ = true;
    // 罚金 = 阶梯倍数 x 本场底注
    escapePenalty_ = escapePenaltyFor(room->historyCount, room->config.ante);
    Account::instance().add(-escapePenalty_);
    rebuildFinalText();
    playFinalSound();
}

void SceneResult::nextRound() {
    if (final_ || betting_) return;
    betting_ = true;
    SoundManager::instance().playBet();
    const int bal = Account::instance().balance();
    chipBar_.setImmediate(bal);
    chipBar_.rollTo(bal - mgr_->room->config.ante, 0.75f);
}

void SceneResult::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (kickPending_) {
        btnDialogOk_.handleEvent(e, win);
        return;
    }
    if (final_) {
        if (!settleSoundDone_ || coinsStarted_) return;
        btnLobby_.handleEvent(e, win);
        chipBar_.handleEvent(e, win);
    } else {
        if (betting_) return;
        btnNext_.handleEvent(e, win);
        btnEscape_.handleEvent(e, win);
        chipBar_.handleEvent(e, win);
    }
}

void SceneResult::update(float dt) {
    chipBar_.update(dt);

    // 局内筹码被外部改动则重判踢出并同步顶部显示
    if (mgr_->room) {
        const int chips = mgr_->room->players[0].chips;
        if (chips != lastChips_) {
            lastChips_ = chips;
            reevaluateKick();
            if (!betting_ && !chipBar_.isRolling()) {
                chipBar_.setImmediate(Account::instance().balance());
            }
        }
    }

    // 下注动画播完进发牌
    if (betting_ && !chipBar_.isRolling()) {
        mgr_->changeTo(SceneId::Deal);
        return;
    }

    // 胜负音播完才出现返回大厅
    if (final_ && !kickPending_ && !settleSoundDone_ &&
        !SoundManager::instance().isPlaying()) {
        settleSoundDone_ = true;
    }

    // 盈亏动画播完自动回大厅
    if (coinsStarted_ && !chipBar_.isRolling()) {
        mgr_->changeTo(SceneId::Lobby);
    }
}

void SceneResult::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);

    if (final_) {
        if (!kickPending_) {
            win.draw(panelRounds_);
            for (int i = 0; i < roundLineCount_; i++) {
                roundLines_[i].draw(win);
            }
            totalLine_.draw(win);
            chipsLine_.draw(win);
            if (settleSoundDone_) btnLobby_.draw(win);
        }
    } else {
        win.draw(panelRows_);
        for (int i = 0; i < mgr_->room->playerCount; i++) {
            playerRows_[i].draw(win);
        }
        btnNext_.draw(win);
        btnEscape_.draw(win);
    }

    chipBar_.draw(win);
    for (int i = 0; i < mgr_->room->playerCount && i < MAX_PLAYERS; i++) {
        avatars_[i].draw(win);
    }

    if (kickPending_) {
        win.draw(overlay_);
        win.draw(dialog_);
        panel_frame::draw(win, sf::FloatRect(dialog_.getPosition(), dialog_.getSize()));
        dialogText_.draw(win);
        dialogSub_.draw(win);
        btnDialogOk_.draw(win);
    }
}
