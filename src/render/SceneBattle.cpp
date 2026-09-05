#include "render/SceneBattle.h"
#include "render/AssetManager.h"
#include "core/Room.h"
#include "core/HandEvaluator.h"
#include <cstdio>
#include <vector>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;

// 各玩家当前道 3 张牌的位置(左上角 x0 + pos*dx, y)
struct SeatPos {
    float x0, y0, dx, scale;   // 3 张横排
    float nameX, nameY;        // 名字位置(居中)
};
// [0]=你(下,大牌) [1]=AI-1(上) [2]=AI-2(左) [3]=AI-3(右)
const SeatPos SEATS[4] = {
    { 470.f, 535.f, 120.f, 0.50f, 640.f, 515.f },  // 你: 100x140(与组牌一致)
    { 535.f,  65.f,  75.f, 0.30f, 640.f,  45.f },  // 上
    {  45.f, 305.f,  65.f, 0.30f, 150.f, 285.f },  // 左
    { 1035.f, 305.f, 65.f, 0.30f, 1140.f, 285.f }  // 右
};
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

    // 玩家名标签
    for (int p = 0; p < 4 && p < playerCount_; p++) {
        const char* dn = (p == 0) ? "你" : room->players[p].name.c_str();
        nameTags_[p].setText(dn);
        nameTags_[p].setCharacterSize(p == 0 ? 22 : 18);
        nameTags_[p].setColor(sf::Color(255, 235, 170));
        nameTags_[p].centerOrigin();
        nameTags_[p].setPosition(sf::Vector2f(SEATS[p].nameX, SEATS[p].nameY));
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
    for (int p = 0; p < 4; p++) {
        for (int pos = 0; pos < 3; pos++) {
            cards_[p][pos].setScale(SEATS[p].scale);
            cards_[p][pos].setPosition(
                sf::Vector2f(SEATS[p].x0 + pos * SEATS[p].dx, SEATS[p].y0));
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
    for (int p = 0; p < 4 && p < playerCount_; p++) {
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
    for (int p = 0; p < 4 && p < playerCount_; p++) {
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

    for (int p = 0; p < 4 && p < playerCount_; p++) {
        nameTags_[p].draw(win);
        for (int pos = 0; pos < 3; pos++) {
            cards_[p][pos].draw(win);
        }
    }

    if (showNext_) {
        // 比完提示文字持续显示(无需额外按钮)
    }
}
