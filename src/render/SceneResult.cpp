#include "render/SceneResult.h"
#include "render/AssetManager.h"
#include "render/SoundManager.h"
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
    btnLobby_.setCallback([this]() { startCoinsPhase(); });   // 播 coins+盈亏动画后自动回大厅

    chipBar_.setPosition(sf::Vector2f(WW - 250.f - 20.f, 16.f));

    // 局内头像: 本人左下角, 他人右侧居中(较小)
    {
        int pc = mgr_->room->playerCount;
        int na = pc - 1;
        if (na < 1) na = 1;
        const float aiX = 1185.f, aiCY = 400.f, aiGap = 80.f, aiR = 22.f;
        float totalH = (na - 1) * aiGap;
        for (int i = 0; i < pc && i < MAX_PLAYERS; i++) {
            avatars_[i].setNickname(mgr_->room->players[i].name);
            avatars_[i].setTexture(AssetManager::instance().avatarTexture(i));   // 头像素材(占位色块)
            if (i == 0) {
                avatars_[i].setRadius(28.f);
                avatars_[i].setMinPlateWidth(150.f);   // 本人: 预留更长昵称空间
                avatars_[i].setSelfStyle(true);        // 名牌与圆心共线, 昵称居中
                avatars_[i].setCenter(sf::Vector2f(78.f, 700.f));
            } else {
                avatars_[i].setRadius(aiR);
                avatars_[i].setCenter(sf::Vector2f(
                    aiX, aiCY - totalH / 2.f + (i - 1) * aiGap));
            }
        }
    }

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
    if (final_) {
        rebuildFinalText();
        playFinalSound();   // 整场结束: 胜负音 + 金币音 + 总盈亏数字跳动
    } else {
        // 非最终局: 筹码框从"下注后余额"滚动到"本局结算后余额"(纯视觉, 不带音效)
        Room* room = mgr_->room.get();
        int ante = room->config.ante;
        int d0 = room->historyCount > 0 ? room->roundHistory[room->historyCount - 1][0] : 0;
        int after = Account::instance().balance();
        chipBar_.setImmediate(after - d0 - ante);
        chipBar_.rollTo(after, 0.8f);
    }
}

// 整场总盈亏(含逃跑罚): 用于整场结束音效判定
int SceneResult::matchTotal() const {
    Room* room = mgr_->room.get();
    int t = -escapePenalty_;
    for (int r = 0; r < room->historyCount; r++) t += room->roundHistory[r][0];
    return t;
}

// 整场结束音效: 总盈亏 >0 胜利音, <0 失败音(0 不播)
// 注意: coins 音效与盈亏数字跳动不在此处——等玩家点击[返回大厅]/home 时才触发,
//       避免与胜负音重叠(见 startCoinsPhase)。
void SceneResult::playFinalSound() {
    int t = matchTotal();
    if (t > 0)      SoundManager::instance().playWin();
    else if (t < 0) SoundManager::instance().playLose();
}

// 点击[返回大厅]/home: 播 coins 金币音(副通道) + 筹码框盈亏数字跳动; 留在本界面,
// 动画播完由 update() 自动回大厅。
void SceneResult::startCoinsPhase() {
    if (coinsStarted_) return;
    coinsStarted_ = true;
    SoundManager::instance().playCoins();
    int t = matchTotal();
    chipBar_.setImmediate(Account::instance().balance() - t);   // 结算前
    chipBar_.rollTo(Account::instance().balance(), 1.0f);       // 滚动到结算后(整场盈亏)
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
        // (胜负音效不在此处播放: 改为整场结束时统一播放, 见 playFinalSound)
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
    tipText_.setText("结算音效播放中…");   // 音效播完后 update() 切换为"点击返回大厅"
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

// 最终局: 居中排布(标题 + 单一面板[每局明细+总盈亏] + 亮红提示 + 返回大厅)
// 面板高度随明细行数动态增长, 保证文本永不越界。
void SceneResult::layoutFinal() {
    const float cx = WW / 2.f;
    float lineH = (roundLineCount_ > 8) ? 24.f : 32.f;
    unsigned fs = (roundLineCount_ > 8) ? 20u : 24u;
    for (int i = 0; i < roundLineCount_; i++) {
        roundLines_[i].setCharacterSize(fs);
    }
    totalLine_.setCharacterSize(fs + 2);
    chipsLine_.setCharacterSize(fs);

    const float sectionGap = 16.f;                 // 明细与总盈亏之间的分隔留白
    float roundsH = roundLineCount_ * lineH;       // 明细区高度(随行数变化)
    float contentH = roundsH + sectionGap + 2 * lineH;
    float panelH = PANEL_PAD * 2 + contentH;       // 面板高度 = 内边距*2 + 内容

    const float titleH = 44.f, tGap = 20.f, tipGap = 28.f, tipH = 26.f, bGap = 22.f, btnH = 60.f;
    float assemblyH = titleH + tGap + panelH + tipGap + tipH + bGap + btnH;
    float top = (WH - assemblyH) / 2.f;
    if (top < 12.f) top = 12.f;

    title_.setPosition(sf::Vector2f(cx, top + titleH / 2.f));
    float panelTop = top + titleH + tGap;

    // 单一面板: 每局明细 + 总盈亏(边框跟随行数)
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

    tipText_.setPosition(sf::Vector2f(cx, panelTop + panelH + tipGap + tipH / 2.f));
    float btnY = panelTop + panelH + tipGap + tipH + bGap;
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
        // 一整场结束: home = 返回大厅(与点击[返回大厅]同流程)
        // 胜负音未播完 / coins 动画进行中: home 不响应(按钮此刻也不显示)
        if (!settleSoundDone_ || coinsStarted_) return;
        startCoinsPhase();
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
    playFinalSound();   // 整场结束(提前逃跑): 同样按总盈亏播放
}

void SceneResult::nextRound() {
    if (final_) return;
    mgr_->changeTo(SceneId::Deal);   // 下一局: 先播发牌动画
}

void SceneResult::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (kickPending_) {
        btnDialogOk_.handleEvent(e, win);
        return;
    }
    if (final_) {
        // 胜负音播放中 / coins 动画进行中: 无交互(不显示返回大厅与home)
        if (!settleSoundDone_ || coinsStarted_) return;
        btnLobby_.handleEvent(e, win);   // 点按钮 = 先播 coins+盈亏动画, 播完自动回大厅
        chipBar_.handleEvent(e, win);    // 筹码图标仍可点(发出 chip 音效)
    } else {
        btnNext_.handleEvent(e, win);
        btnEscape_.handleEvent(e, win);
        chipBar_.handleEvent(e, win);
    }
}

void SceneResult::update(float dt) {
    chipBar_.update(dt);   // 筹码框数字滚动(结算收账/盈亏动画)

    // 胜负音播完的瞬间: 出现[返回大厅]按钮与 home(位置不变), 提示同步切换
    if (final_ && !kickPending_ && !settleSoundDone_ &&
        !SoundManager::instance().isPlaying()) {
        settleSoundDone_ = true;
        tipText_.setText("点击下方 [返回大厅] 回到人数选择");
    }

    // coins 盈亏动画播完 -> 自动返回大厅
    if (coinsStarted_ && !chipBar_.isRolling()) {
        mgr_->changeTo(SceneId::Lobby);
    }
}

void SceneResult::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);

    if (final_) {
        if (!kickPending_) {
            // 单一面板: 每局明细 + 总盈亏
            win.draw(panelRounds_);
            for (int i = 0; i < roundLineCount_; i++) {
                roundLines_[i].draw(win);
            }
            totalLine_.draw(win);
            chipsLine_.draw(win);
            tipText_.draw(win);
            if (settleSoundDone_) btnLobby_.draw(win);   // 胜负音播完才出现(原位置)
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

    chipBar_.draw(win);
    for (int i = 0; i < mgr_->room->playerCount && i < MAX_PLAYERS; i++) {
        avatars_[i].draw(win);   // 局内头像: 本人左下 / 他人右中
    }

    if (kickPending_) {
        win.draw(overlay_);
        win.draw(dialog_);
        dialogText_.draw(win);
        dialogSub_.draw(win);
        btnDialogOk_.draw(win);
    }
}
