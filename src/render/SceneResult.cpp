#include "render/SceneResult.h"
#include "render/AssetManager.h"
#include "render/Account.h"
#include "core/Room.h"
#include <cstdio>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;
const sf::Color C_WIN(0, 200, 90);     // 盈利鲜绿
const sf::Color C_LOSE(255, 70, 70);   // 亏损鲜红
const sf::Color C_GOLD(255, 215, 0);
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
    title_.setPosition(sf::Vector2f(WW / 2.f, 48.f));

    for (int i = 0; i < MAX_PLAYERS; i++) {
        playerRows_[i].setCharacterSize(26);
        playerRows_[i].centerOrigin();
        playerRows_[i].setPosition(sf::Vector2f(WW / 2.f, 160.f + i * 56.f));
    }
    detailText_.setCharacterSize(24);
    detailText_.setColor(sf::Color(240, 240, 240));
    detailText_.centerOrigin();
    detailText_.setPosition(sf::Vector2f(WW / 2.f, 170.f));

    tipText_.setCharacterSize(18);
    tipText_.setColor(sf::Color(220, 220, 220));
    tipText_.centerOrigin();
    tipText_.setPosition(sf::Vector2f(WW / 2.f, 610.f));

    btnNext_.setText("下一局");
    btnNext_.setPosition(sf::Vector2f(290.f, 660.f));
    btnNext_.setSize(sf::Vector2f(320.f, 60.f));
    btnNext_.setCallback([this]() { nextRound(); });

    btnEscape_.setText("逃跑");
    btnEscape_.setPosition(sf::Vector2f(670.f, 660.f));
    btnEscape_.setSize(sf::Vector2f(320.f, 60.f));
    btnEscape_.setCallback([this]() { escape(); });

    btnLobby_.setText("返回大厅");
    btnLobby_.setPosition(sf::Vector2f(430.f, 690.f));
    btnLobby_.setSize(sf::Vector2f(420.f, 64.f));
    btnLobby_.setCallback([this]() { mgr_->changeTo(SceneId::Lobby); });

    chipBar_.setPosition(sf::Vector2f(WW - 250.f - 20.f, 16.f));

    // 踢出弹窗样式(弹窗宽 680, 文案两行)
    overlay_.setSize(sf::Vector2f(WW, WH));
    overlay_.setFillColor(sf::Color(0, 0, 0, 160));
    dialog_.setSize(sf::Vector2f(680.f, 250.f));
    dialog_.setPosition(sf::Vector2f((WW - 680.f) / 2.f, (WH - 250.f) / 2.f));
    dialog_.setFillColor(sf::Color(30, 40, 70));
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

    // 账号同步: 真人(下标0)本局盈亏 = 历史末行(真实记录, 不在此补充)
    if (room->historyCount > 0) {
        int d0 = room->roundHistory[room->historyCount - 1][0];
        Account::instance().add(d0);
    }

    // 踢出判定: 本局结算后, 若真人牌桌筹码不足以支付下一局底注(3×底注),
    // 直接踢出本场(不扣逃跑费); 破产补充统一回到大厅再检测
    if (!room->isFinished()) {
        int needNext = 3 * room->config.ante;   // 下一局需 3 份底注
        if (room->players[0].chips < needNext) {
            kickPending_ = true;
            final_ = true;   // 本场结束(被踢出), 不显示下一局/逃跑
            char t[128];
            std::snprintf(t, sizeof(t), "您的筹码 (%d) 不足以支付下一局底注 (%d)，\n您已被踢出本场对局",
                          room->players[0].chips, needNext);
            dialogText_.setText(t);   // 居中模式下 setText 自动保持居中
            dialogSub_.setText("本次不扣除逃跑费用 · 点击确定返回大厅");
        }
    }
}

void SceneResult::refreshRows() {
    Room* room = mgr_->room.get();
    // 标题
    char t[48];
    std::snprintf(t, sizeof(t), "第 %d 局 结算", room->currentRound);
    title_.setText(t);

    if (!final_) {
        // 非最终局: 每家 筹码 (本局盈亏)
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
    } else {
        rebuildFinalText();
    }
}

void SceneResult::rebuildFinalText() {
    Room* room = mgr_->room.get();
    std::string s;
    char line[96];
    int total = 0;
    // 每局明细(只显示真人 = 下标0)
    for (int r = 0; r < room->historyCount; r++) {
        int d = room->roundHistory[r][0];
        total += d;
        std::snprintf(line, sizeof(line), "第 %d 局: %+d\n", r + 1, d);
        s += line;
    }
    if (escapePenalty_ != 0) {
        total -= escapePenalty_;
        std::snprintf(line, sizeof(line), "逃跑罚: -%d\n", escapePenalty_);
        s += line;
    }
    std::snprintf(line, sizeof(line), "总盈亏: %+d\n当前筹码: %d",
                  total, Account::instance().balance());
    s += line;
    detailText_.setText(s);
    detailText_.setColor(total >= 0 ? C_WIN : C_LOSE);

    char t[64];
    std::snprintf(t, sizeof(t), "%s", escapePenalty_ ? "本场提前结束" : "本场结束");
    title_.setText(t);
    tipText_.setText("点击下方 [返回大厅] 回到人数选择");
}

void SceneResult::confirmKickOut() {
    kickPending_ = false;
    mgr_->changeTo(SceneId::Lobby);   // 回大厅(破产补充在大厅检测)
}

void SceneResult::escape() {
    if (final_) return;
    final_ = true;
    escapePenalty_ = 100;
    // 罚 100 记入账号
    Account::instance().add(-100);
    rebuildFinalText();
}

void SceneResult::nextRound() {
    if (final_) return;   // 最终结算无下一局
    mgr_->changeTo(SceneId::Arrange);   // 开局由 SceneArrange 构造统一完成
}

void SceneResult::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (kickPending_) {
        btnDialogOk_.handleEvent(e, win);   // 踢出弹窗只响应确定
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
        // 被踢出时只显示弹窗, 不画结算明细
        if (!kickPending_) {
            detailText_.draw(win);
            btnLobby_.draw(win);
        }
    } else {
        for (int i = 0; i < mgr_->room->playerCount; i++) {
            playerRows_[i].draw(win);
        }
        btnNext_.draw(win);
        btnEscape_.draw(win);
    }
    tipText_.draw(win);
    chipBar_.draw(win, Account::instance().balance());

    if (kickPending_) {
        win.draw(overlay_);
        win.draw(dialog_);
        dialogText_.draw(win);
        dialogSub_.draw(win);
        btnDialogOk_.draw(win);
    }
}
