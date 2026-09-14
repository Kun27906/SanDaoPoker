#include "render/SceneBattle.h"
#include "render/AssetManager.h"
#include "core/Room.h"
#include "core/HandEvaluator.h"
#include <cstdio>
#include <vector>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;

// ====== 座位表(按用户指定编号) ======
// 1=你(下方大牌)  2=右中下  3=右中上  4=上方靠右  5=上方靠左  6=左中
// 每人一座: 牌组(3 张横排) + 头像昵称位(ax,ay)
// 约束: 所有座位的牌组顶边与头像顶边都不得高于标题下边沿 TITLE_BOTTOM(编译期 static_assert 护栏)
struct Seat {
    float cx, cy, dx, s;   // 牌组: 中张左缘 x / 中线 y / 张间距 / 缩放
    float ax, ay;          // 头像圆心(名牌由 Avatar 自绘)
};
constexpr float TITLE_BOTTOM = 50.f;   // 标题(第 X 局·比牌 @y22)下边沿 + 余量
constexpr float CARD_UNIT_W = 200.f;   // 单张牌素材宽
constexpr float CARD_UNIT_H = 280.f;   // 单张牌素材高

constexpr Seat SEATS[7] = {            // 下标 = 座位号(0 空置)
    { 0.f,     0.f,    0.f,   0.f,    0.f,    0.f    },   // 0 占位
    { 588.f,   660.f,  100.f, 0.52f,  300.f,  700.f  },   // 1 你(下方, 大牌)
    { 1060.f,  480.f,  58.f,  0.40f,  1010.f, 575.f  },   // 2 右中下(头像在牌下)
    { 1060.f,  300.f,  58.f,  0.40f,  1010.f, 388.f  },   // 3 右中上(头像在牌下)
    { 700.f,   150.f,  58.f,  0.40f,  880.f,  150.f  },   // 4 上方靠右(头像在牌右侧)
    { 330.f,   150.f,  58.f,  0.40f,  180.f,  150.f  },   // 5 上方靠左(头像在牌左侧)
    { 180.f,   340.f,  58.f,  0.40f,  135.f,  432.f  },   // 6 左中(头像在牌下)
};

// 编译期护栏: 任何座位的牌组顶 / 头像顶 都不得高于标题下边沿
constexpr bool layoutBelowTitle() {
    for (int i = 1; i <= 6; ++i) {
        const Seat& st = SEATS[i];
        if (st.cy - CARD_UNIT_H * 0.5f * st.s < TITLE_BOTTOM) return false;
        if (st.ay - 28.f < TITLE_BOTTOM) return false;   // 28 = 头像圆环半径+边框余量
    }
    return true;
}
static_assert(layoutBelowTitle(), "seat layout must stay below the title bottom edge");

// 人数 -> 座位号序列(玩家 p 使用第 p 个座位号; 用户指定映射)
constexpr int SEAT_ORDER_2[2] = { 1, 4 };
constexpr int SEAT_ORDER_3[3] = { 1, 4, 5 };
constexpr int SEAT_ORDER_4[4] = { 1, 2, 3, 6 };
constexpr int SEAT_ORDER_5[5] = { 1, 2, 3, 5, 6 };
constexpr int SEAT_ORDER_6[6] = { 1, 2, 3, 4, 5, 6 };

int seatNumberOf(int pc, int p) {
    switch (pc) {
        case 2: return (p < 2) ? SEAT_ORDER_2[p] : 0;
        case 3: return (p < 3) ? SEAT_ORDER_3[p] : 0;
        case 4: return (p < 4) ? SEAT_ORDER_4[p] : 0;
        case 5: return (p < 5) ? SEAT_ORDER_5[p] : 0;
        case 6: return (p < 6) ? SEAT_ORDER_6[p] : 0;
    }
    return 0;
}

const Seat* seatFor(int pc, int p) {
    const int n = seatNumberOf(pc, p);
    return (n >= 1 && n <= 6) ? &SEATS[n] : nullptr;
}
const char* LINE_NAMES[3] = {"头道", "中道", "尾道"};
constexpr float BACK_SHOW = 0.8f;     // 全家牌背亮相时间
constexpr float PER_HOLD = 1.0f;      // 逐家翻牌: 每家翻完停留 1 秒再翻下一家
constexpr float HOLD_TIME = 2.5f;     // 本道结果(赢家/牌型)停留时间
}

SceneBattle::SceneBattle(SceneManager* mgr) : mgr_(mgr) {
    if (const sf::Texture* bg = AssetManager::instance().background()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

    Room* room = mgr_->room.get();
    playerCount_ = room ? room->playerCount : 0;

    // 标题
    char buf[64];
    std::snprintf(buf, sizeof(buf), "第 %d 局 · 比牌", room->currentRound);
    title_.setText(buf);
    title_.setCharacterSize(30);
    title_.setColor(sf::Color(255, 215, 0));
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 22.f));

    // 头像昵称系统(替换原"纯昵称"标签): 每人一座, 与牌组同座
    for (int p = 0; p < playerCount_; p++) {
        const Seat* st = seatFor(playerCount_, p);
        if (!st) continue;
        Avatar& av = seatAvatars_[p];
        const bool isSelf = (p == 0);
        av.setRadius(isSelf ? 26.f : 22.f);
        av.setSelfStyle(isSelf);                 // 本人: 名牌与圆心共线; 他人: 名牌在圆环右下
        if (isSelf) av.setMinPlateWidth(150.f);  // 本人: 预留更长昵称空间
        av.setTexture(AssetManager::instance().avatarTexture(p));   // 各玩家不同头像(取模循环)
        av.setNickname(room->players[p].name);
        av.setCenter(sf::Vector2f(st->ax, st->ay));
    }

    // 中央信息(金色大字)
    info_.setCharacterSize(32);
    info_.setColor(sf::Color(255, 200, 60));
    info_.centerOrigin();
    info_.setPosition(sf::Vector2f(640.f, 235.f));

    lineTag_.setCharacterSize(20);
    lineTag_.setColor(sf::Color(255, 230, 150));
    lineTag_.centerOrigin();
    lineTag_.setPosition(sf::Vector2f(640.f, 285.f));

    // 局内本人头像已由座位表(1 号位)统一承载, 不再单独绘制

    // 牌精灵初始化位置(内容由 loadLine 装载)
    for (int p = 0; p < playerCount_; p++) {
        const Seat* st = seatFor(playerCount_, p);
        if (!st) continue;
        for (int pos = 0; pos < 3; pos++) {
            cards_[p][pos].setScale(st->s);
            cards_[p][pos].setPosition(
                sf::Vector2f(st->cx - st->dx + pos * st->dx, st->cy - 140.f * st->s));
        }
    }

    // 从头道开始:先展示牌背(三道比完自动 3 秒进入结算, 无按钮)
    char tag0[32];
    std::snprintf(tag0, sizeof(tag0), "%s · 1/3", LINE_NAMES[0]);
    lineTag_.setText(tag0);
    info_.setText("比牌开始!");
    loadLine(0, false);
}

void SceneBattle::loadLine(int lineId, bool faceUp) {
    Room* room = mgr_->room.get();
    for (int p = 0; p < playerCount_; p++) {
        const Seat* st = seatFor(playerCount_, p);
        if (!st) continue;
        for (int pos = 0; pos < 3; pos++) {
            Card c = room->players[p].lines[lineId][pos];
            cards_[p][pos].setCard(c);
            cards_[p][pos].setFaceUp(faceUp);
        }
    }
}

// 翻开某一位玩家的当前道 3 张(逐家翻牌: 按座位号由小到大, 即玩家下标 0,1,2,...)
void SceneBattle::flipPlayer(int p) {
    const Seat* st = seatFor(playerCount_, p);
    if (!st) return;
    for (int pos = 0; pos < 3; pos++) {
        cards_[p][pos].setFaceUp(true);
    }
}

// 最后一家翻完并停留结束后: 计算并显示本道赢家 + 牌型(纯展示, 不结算筹码)
void SceneBattle::revealWinner() {
    Room* room = mgr_->room.get();
    int winners[MAX_PLAYERS];
    int cnt = Round::findWinners(room->players, playerCount_, showLine_, winners);
    char buf[96];
    if (cnt == 1) {
        int w = winners[0];
        std::vector<Card> three = {
            room->players[w].lines[showLine_][0],
            room->players[w].lines[showLine_][1],
            room->players[w].lines[showLine_][2]
        };
        HandResult r = HandEvaluator::evaluate(three);
        const char* dn = (w == 0) ? "你" : room->players[w].name.c_str();
        std::snprintf(buf, sizeof(buf), "%s: %s 赢 (%s)",
                      LINE_NAMES[showLine_], dn, r.name().c_str());
    } else {
        std::snprintf(buf, sizeof(buf), "%s: %d 家打平!", LINE_NAMES[showLine_], cnt);
    }
    info_.setText(buf);
}

void SceneBattle::advance() {
    showLine_++;
    if (showLine_ >= 3) {
        // 三道全部比完: 等待 3 秒后自动进入结算
        info_.setText("比牌完成, 即将进入结算...");
        lineTag_.setText("三组比完");
        showNext_ = true;
        phase_ = 3;      // 阶段 3 = 等进结算(新编号: 2 让给"本道结果停留")
        timer_ = 0.f;
        return;
    }
    char tag[32];
    std::snprintf(tag, sizeof(tag), "%s · %d/3", LINE_NAMES[showLine_], showLine_ + 1);
    lineTag_.setText(tag);
    info_.setText("");
    loadLine(showLine_, false);   // 下一道先牌背
}

void SceneBattle::handleEvent(const sf::Event&, const sf::RenderWindow&) {
    // 自动流程, 无交互按钮
}

void SceneBattle::update(float dt) {
    // 阶段 3: 三组比完, 等 3 秒自动进结算
    if (phase_ == 3) {
        timer_ += dt;
        if (timer_ >= 3.f) mgr_->changeTo(SceneId::Result);
        return;
    }

    timer_ += dt;
    if (phase_ == 0) {
        // 全家牌背亮相 -> 从 1 号座位(本人)开始逐家翻牌
        if (timer_ >= BACK_SHOW) {
            timer_ = 0.f;
            phase_ = 1;
            flipIndex_ = 0;
            flipPlayer(flipIndex_);
        }
    } else if (phase_ == 1) {
        // 每家翻完停留 PER_HOLD(1 秒)再翻下一家; 最后一家停留结束 -> 宣布赢家
        if (timer_ >= PER_HOLD) {
            timer_ = 0.f;
            flipIndex_++;
            if (flipIndex_ < playerCount_) {
                flipPlayer(flipIndex_);
            } else {
                revealWinner();
                phase_ = 2;
            }
        }
    } else if (phase_ == 2) {
        // 本道结果停留 HOLD_TIME -> 进入下一道(或"三组比完")
        if (timer_ >= HOLD_TIME) {
            timer_ = 0.f;
            phase_ = 0;
            advance();
        }
    }
}

void SceneBattle::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    info_.draw(win);
    lineTag_.draw(win);

    for (int p = 0; p < playerCount_; p++) {
        const Seat* st = seatFor(playerCount_, p);
        if (!st) continue;
        for (int pos = 0; pos < 3; pos++) {
            cards_[p][pos].draw(win);
        }
        seatAvatars_[p].draw(win);   // 该玩家头像 + 昵称名牌(与牌组同座)
    }

    if (showNext_) {
        // 比完提示文字持续显示(无需额外按钮)
    }
}
