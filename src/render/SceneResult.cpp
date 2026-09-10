#include "render/SceneResult.h"
#include "render/AssetManager.h"
#include "render/Account.h"
#include "core/Room.h"
#include <cstdio>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;
const sf::Color C_WIN(0, 200, 90);       // 盈利鲜绿
const sf::Color C_LOSE(255, 70, 70);     // 亏损鲜红
const sf::Color C_GOLD(255, 215, 0);
const sf::Color C_ALERT(255, 80, 80);    // 提示行亮红
const sf::Color C_PANEL_BG(30, 40, 70);  // 面板底色(与弹窗一致)
constexpr float PANEL_W = 620.f;
constexpr float PANEL_PAD = 24.f;
}

SceneResult::SceneResult(SceneManager* mgr) : mgr_(mgr) {
    if (const sf::Texture* bg = AssetManager::instance().background()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

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

    tipText_.setCharacterSize(18);
    tipText_.setColor(C_ALERT);        // 提示行统一亮红
    tipText_.centerOrigin();

    // 高亮面板(与弹窗同色: 深蓝底 + 金边)
    for (sf::RectangleShape* p : {&panelRows_, &panelRounds_, &panelTotal_}) {
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
    btnLobby_.setCallback([this]() { mgr_->changeTo(SceneId::Lobby); });

    chipBar_.setPosition(sf::Vector2f(WW - 250.f - 20.f, 16.f));

    // 踢出弹窗样式(弹窗宽 680, 文案两行)
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

    // 结算 + 账号同步 + 踢出判定 + 最终模式判定
    settleAndSync();
    final_ = mgr_->room->isFinished();
    if (!final_ && !kickPending_) refreshRows();
    if (final_) rebuildFinalText();
}

void SceneResult::settleAndSync() {
    if (synced_) return;
    synced_ = true;
    Room* room = mgr_->room.get();
    room->settleRound();   // 内部记录本局盈亏到 roundHistory

    // 账号同步: 真人(下标0)本局盈亏 = 历史末行
    if (room->historyCount > 0) {
        int d0 = room->roundHistory[room->historyCount - 1][0];
        Account::instance().add(d0);
    }

    // 踢出判定: 本局结算后筹码不足下一局个人注金 -> 踢出(不扣逃跑费)
    if (!room->isFinished()) {
        int needNext = room->config.ante;
        if (room->players[0].chips < needNext) {
            kickPending_ = true;
            final_ = true;
            char t[128];
            std::snprintf(t, sizeof(t), "您的筹码 (%d) 不足以支付下一局底注 (%d)，\n您已被踢出本场对局",
                          room->players[0].chips, needNext);
            dialogText_.setText(t);
            dialogSub_.setText("本次不扣除逃跑费用 · 点击确定返回大厅");
        }
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
        tipText_.setText("点[下一局]继续本场 · 点[逃跑]立即结束本场(罚 100)");
        layoutRows();
    } else {
        rebuildFinalText();
    }
}

void SceneResult::rebuildFinalText() {
    Room* room = mgr_->room.get();
    int total = 0;
    int n = 0;
    // 每局盈亏:逐行独立着色(盈利绿/亏损红)
    for (int r = 0; r < room->historyCount && n < MAX_ROUND_LINES; r++) {
        int d = room->roundHistory[r][0];
        total += d;
        char line[96];
        std::snprintf(line, sizeof(line), "第 %d 局:  %+d", r + 1, d);
        roundLines_[n].setText(line);
        roundLines_[n].setColor(d > 0 ? C_WIN : (d < 0 ? C_LOSE : sf::Color(235, 235, 235)));
        n++;
    }
    // 逃跑罚(红)
    if (escapePenalty_ != 0 && n < MAX_ROUND_LINES) {
        total -= escapePenalty_;
        char line[96];
        std::snprintf(line, sizeof(line), "逃跑罚:  -%d", escapePenalty_);
        roundLines_[n].setText(line);
        roundLines_[n].setColor(C_LOSE);
        n++;
    }
    roundLineCount_ = n;

    // 总盈亏(红/绿) + 当前筹码
    char buf[96];
    std::snprintf(buf, sizeof(buf), "总盈亏:  %+d", total);
    totalLine_.setText(buf);
    totalLine_.setColor(total >= 0 ? C_WIN : C_LOSE);
    std::snprintf(buf, sizeof(buf), "当前筹码:  %d", Account::instance().balance());
    chipsLine_.setText(buf);

    title_.setText(escapePenalty_ ? "本场提前结束" : "本场结束");
    tipText_.setText("点击下方 [返回大厅] 回到人数选择");
    tipText_.setColor(C_ALERT);   // 亮红

    layoutFinal();
}

// 非最终局: 居中排布(标题 + 各家筹码面板 + 亮红提示 + 两个按钮)
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

    tipText_.setPosition(sf::Vector2f(cx, blockTop + rowsH + tipGap + tipH / 2.f));

    float btnY = blockTop + rowsH + tipGap + tipH + bGap;
    const float bw = 300.f, bg = 40.f;
    float left = cx - (bw * 2.f + bg) / 2.f;
    btnNext_.setPosition(sf::Vector2f(left, btnY));
    btnNext_.setSize(sf::Vector2f(bw, btnH));
    btnEscape_.setPosition(sf::Vector2f(left + bw + bg, btnY));
    btnEscape_.setSize(sf::Vector2f(bw, btnH));
}

// 最终局: 居中排布(标题 + 每局结算面板 + 总盈亏结算面板 + 亮红提示 + 返回大厅)
void SceneResult::layoutFinal() {
    const float cx = WW / 2.f;
    float lineH = (roundLineCount_ > 8) ? 24.f : 32.f;
    unsigned fs = (roundLineCount_ > 8) ? 20u : 24u;
    for (int i = 0; i < roundLineCount_; i++) {
        roundLines_[i].setCharacterSize(fs);
    }
    totalLine_.setCharacterSize(fs + 2);
    chipsLine_.setCharacterSize(fs);

    const float gap = 20.f;
    float roundsH = PANEL_PAD * 2 + roundLineCount_ * lineH;
    float totalH = PANEL_PAD * 2 + 2 * lineH;
    float blockH = roundsH + gap + totalH;

    const float titleH = 44.f, tGap = 20.f, tipGap = 28.f, tipH = 26.f, bGap = 22.f, btnH = 60.f;
    float assemblyH = titleH + tGap + blockH + tipGap + tipH + bGap + btnH;
    float top = (WH - assemblyH) / 2.f;
    if (top < 12.f) top = 12.f;

    title_.setPosition(sf::Vector2f(cx, top + titleH / 2.f));
    float blockTop = top + titleH + tGap;

    // 每局结算面板
    panelRounds_.setSize(sf::Vector2f(PANEL_W, roundsH));
    panelRounds_.setPosition(sf::Vector2f(cx - PANEL_W / 2.f, blockTop));
    for (int i = 0; i < roundLineCount_; i++) {
        roundLines_[i].setPosition(sf::Vector2f(cx, blockTop + PANEL_PAD + i * lineH + lineH / 2.f));
    }

    // 总盈亏结算面板
    float p2Top = blockTop + roundsH + gap;
    panelTotal_.setSize(sf::Vector2f(PANEL_W, totalH));
    panelTotal_.setPosition(sf::Vector2f(cx - PANEL_W / 2.f, p2Top));
    totalLine_.setPosition(sf::Vector2f(cx, p2Top + PANEL_PAD + lineH / 2.f));
    chipsLine_.setPosition(sf::Vector2f(cx, p2Top + PANEL_PAD + lineH + lineH / 2.f));

    tipText_.setPosition(sf::Vector2f(cx, blockTop + blockH + tipGap + tipH / 2.f));
    float btnY = blockTop + blockH + tipGap + tipH + bGap;
    btnLobby_.setPosition(sf::Vector2f(cx - 210.f, btnY));
    btnLobby_.setSize(sf::Vector2f(420.f, btnH));
}

void SceneResult::confirmKickOut() {
    kickPending_ = false;
    mgr_->changeTo(SceneId::Lobby);   // 回大厅(破产补充在大厅检测)
}

void SceneResult::onHomePressed() {
    if (kickPending_) return;              // 踢出弹窗期间不响应
    if (final_) {
        mgr_->changeTo(SceneId::Lobby);    // 一整场结束: home = 返回大厅
    } else {
        escape();                          // 一局结束后: home = 逃跑
    }
}

void SceneResult::escape() {
    if (final_) return;
    final_ = true;
    escapePenalty_ = 100;
    Account::instance().add(-100);
    rebuildFinalText();
}

void SceneResult::nextRound() {
    if (final_) return;
    mgr_->changeTo(SceneId::Arrange);
}

void SceneResult::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (kickPending_) {
        btnDialogOk_.handleEvent(e, win);
        return;
    }
    if (final_) {
        btnLobby_.handleEvent(e, win);
    } else {
        btnNext_.handleEvent(e, win);
        btnEscape_.handleEvent(e, win);
    }
}

void SceneResult::update(float) {}

void SceneResult::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);

    if (final_) {
        if (!kickPending_) {
            // 每局结算块
            win.draw(panelRounds_);
            for (int i = 0; i < roundLineCount_; i++) {
                roundLines_[i].draw(win);
            }
            // 总盈亏结算块
            win.draw(panelTotal_);
            totalLine_.draw(win);
            chipsLine_.draw(win);
            tipText_.draw(win);
            btnLobby_.draw(win);
        }
    } else {
        // 本局结算块
        win.draw(panelRows_);
        for (int i = 0; i < mgr_->room->playerCount; i++) {
            playerRows_[i].draw(win);
        }
        tipText_.draw(win);
        btnNext_.draw(win);
        btnEscape_.draw(win);
    }

    chipBar_.draw(win, Account::instance().balance());

    if (kickPending_) {
        win.draw(overlay_);
        win.draw(dialog_);
        dialogText_.draw(win);
        dialogSub_.draw(win);
        btnDialogOk_.draw(win);
    }
}
