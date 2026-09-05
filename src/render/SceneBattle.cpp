#include "render/SceneBattle.h"
#include "render/AssetManager.h"
#include "core/Room.h"
#include "core/HandEvaluator.h"
#include <cstdio>
#include <vector>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;

// 各玩家座位参数: cx,cy=牌组中心; 3 张横排间距 dx; s=缩放
// 支持 2~6 人: 下标0=你(players[0], 底部大牌), 其余按 p 顺序落座
struct Seat {
    float cx, cy, dx, s;
};
const Seat* seatFor(int pc, int p) {
    static const Seat s2[2] = {
        { 640.f, 620.f, 128.f, 0.68f },  // 你
        { 640.f, 110.f, 112.f, 0.58f }   // 上
    };
    static const Seat s3[3] = {
        { 640.f, 625.f, 122.f, 0.64f },  // 你
        { 330.f, 120.f, 104.f, 0.52f },  // 左上
        { 950.f, 120.f, 104.f, 0.52f }   // 右上
    };
    static const Seat s4[4] = {
        { 640.f, 630.f, 120.f, 0.62f },  // 你
        { 640.f, 100.f, 96.f, 0.48f },   // 上
        { 195.f, 335.f, 96.f, 0.48f },   // 左
        { 1085.f, 335.f, 96.f, 0.48f }   // 右
    };
    static const Seat s5[5] = {
        { 640.f, 640.f, 104.f, 0.52f },  // 你
        { 640.f, 100.f, 80.f, 0.40f },   // 上中
        { 245.f, 170.f, 80.f, 0.40f },   // 左上
        { 1035.f, 170.f, 80.f, 0.40f },  // 右上
        { 155.f, 480.f, 80.f, 0.40f }    // 左下
    };
    static const Seat s6[6] = {
        { 640.f, 645.f, 96.f, 0.48f },   // 你
        { 640.f, 95.f, 72.f, 0.36f },    // 上中
        { 275.f, 160.f, 72.f, 0.36f },   // 左上
        { 1005.f, 160.f, 72.f, 0.36f },  // 右上
        { 155.f, 490.f, 72.f, 0.36f },   // 左下
        { 1125.f, 490.f, 72.f, 0.36f }   // 右下
    };
    if (p < 0) return nullptr;
    switch (pc) {
        case 2: return (p < 2) ? &s2[p] : nullptr;
        case 3: return (p < 3) ? &s3[p] : nullptr;
        case 4: return (p < 4) ? &s4[p] : nullptr;
        case 5: return (p < 5) ? &s5[p] : nullptr;
        case 6: return (p < 6) ? &s6[p] : nullptr;
    }
    return nullptr;
}
const char* LINE_NAMES[3] = {"头道", "中道", "尾道"};
constexpr float FLIP_DELAY = 0.8f;    // 牌背展示时间
constexpr float HOLD_TIME = 2.5f;     // 结果停留时间
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

    // 玩家名标签(2~6 人全部落座)
    for (int p = 0; p < playerCount_; p++) {
        const Seat* st = seatFor(playerCount_, p);
        if (!st) continue;
        const char* dn = (p == 0) ? "你" : room->players[p].name.c_str();
        nameTags_[p].setText(dn);
        nameTags_[p].setCharacterSize(p == 0 ? 22 : 16);
        nameTags_[p].setColor(sf::Color(255, 235, 170));
        nameTags_[p].centerOrigin();
        nameTags_[p].setPosition(sf::Vector2f(st->cx, st->cy - 140.f * st->s - 24.f));
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

void SceneBattle::flipUp() {
    Room* room = mgr_->room.get();
    // 翻正当前道的牌
    for (int p = 0; p < playerCount_; p++) {
        const Seat* st = seatFor(playerCount_, p);
        if (!st) continue;
        for (int pos = 0; pos < 3; pos++) {
            cards_[p][pos].setFaceUp(true);
        }
    }

    // 计算该道赢家 + 牌型(纯展示)
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
        phase_ = 2;
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
    if (phase_ == 2) {
        // 比完等待 3 秒自动进结算
        timer_ += dt;
        if (timer_ >= 3.f) {
            mgr_->changeTo(SceneId::Result);
        }
        return;
    }
    timer_ += dt;
    if (phase_ == 0 && timer_ >= FLIP_DELAY) {
        timer_ = 0.f;
        phase_ = 1;
        flipUp();
    } else if (phase_ == 1 && timer_ >= HOLD_TIME) {
        timer_ = 0.f;
        phase_ = 0;
        advance();
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
        nameTags_[p].draw(win);
        for (int pos = 0; pos < 3; pos++) {
            cards_[p][pos].draw(win);
        }
    }

    if (showNext_) {
        // 比完提示文字持续显示(无需额外按钮)
    }
}
