#include "render/SceneBattle.h"
#include "render/AssetManager.h"
#include "core/Room.h"
#include "core/HandEvaluator.h"
#include <cstdio>
#include <vector>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;

// 每家的牌面布局: [玩家] 起始点 + scale + 行距
struct Layout {
    float x0, y0;        // 第一张牌(头道第1张)的左上角
    float scale;         // 牌缩放
    float dx, dy;        // 列距/行距
    float nameX, nameY;  // 玩家名位置(居中)
};
// 上=players[1] 左=players[2] 右=players[3] 下=players[0]
const Layout LAYOUTS[4] = {
    { 470.f, 585.f, 0.34f, 74.f, 100.f, 640.f, 565.f },  // [0] 下: 你
    { 520.f, 70.f,  0.24f, 56.f, 76.f,  640.f, 45.f  },  // [1] 上: AI-1
    { 70.f,  300.f, 0.24f, 56.f, 76.f,  135.f, 278.f },  // [2] 左: AI-2
    { 1050.f, 300.f, 0.24f, 56.f, 76.f, 1115.f, 278.f }  // [3] 右: AI-3
};

const char* LINE_NAMES[3] = {"头道", "中道", "尾道"};
}

SceneBattle::SceneBattle(SceneManager* mgr) : mgr_(mgr) {
    // 背景
    if (const sf::Texture* bg = AssetManager::instance().background()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

    title_.setText("比牌");
    title_.setCharacterSize(36);
    title_.setColor(sf::Color::White);
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 24.f));

    // 四家牌面:初始全部牌背
    Room* room = mgr_->room.get();
    const int pc = room ? room->playerCount : 0;
    for (int p = 0; p < 4 && p < pc; p++) {
        const Layout& L = LAYOUTS[p];
        for (int line = 0; line < 3; line++) {
            for (int pos = 0; pos < 3; pos++) {
                CardSprite& cs = cardSprites_[p][line][pos];
                cs.setScale(L.scale);
                cs.setPosition(sf::Vector2f(L.x0 + pos * L.dx, L.y0 + line * L.dy));
                cs.setFaceUp(false);   // 初始牌背,逐道揭示
            }
        }
        // 名字与筹码
        const char* displayName = (p == 0) ? "你" : room->players[p].name.c_str();
        nameTags_[p].setText(displayName);
        nameTags_[p].setCharacterSize(20);
        nameTags_[p].setColor(sf::Color::White);
        nameTags_[p].centerOrigin();
        nameTags_[p].setPosition(sf::Vector2f(L.nameX, L.nameY));

        char chip[32];
        std::snprintf(chip, sizeof(chip), "筹码:%d", room->players[p].chips);
        chipTags_[p].setText(chip);
        chipTags_[p].setCharacterSize(16);
        chipTags_[p].setColor(sf::Color(255, 230, 150));
        chipTags_[p].centerOrigin();
        chipTags_[p].setPosition(sf::Vector2f(L.nameX, L.nameY + 24.f));
    }

    // 中央信息
    info_.setText("正在比牌...");
    info_.setCharacterSize(26);
    info_.setColor(sf::Color(255, 255, 255));
    info_.centerOrigin();
    info_.setPosition(sf::Vector2f(640.f, 380.f));

    // 查看结算按钮(动画完成后可用)
    btnNext_.setText("查看结算");
    btnNext_.setPosition(sf::Vector2f(470.f, 520.f));
    btnNext_.setSize(sf::Vector2f(340.f, 60.f));
    btnNext_.setCallback([this]() { mgr_->changeTo(SceneId::Result); });
}

void SceneBattle::revealNext() {
    revealLine_++;
    if (revealLine_ >= 3) {
        // 三道全部揭示完毕
        info_.setText("比牌完成! 点击[查看结算]");
        showNext_ = true;
        return;
    }

    // 翻开当前道的所有玩家牌
    Room* room = mgr_->room.get();
    const int pc = room ? room->playerCount : 0;
    for (int p = 0; p < 4 && p < pc; p++) {
        for (int pos = 0; pos < 3; pos++) {
            // 从 room 中读取真实牌面(Arrange 阶段已摆好)
            Card c = room->players[p].lines[revealLine_][pos];
            cardSprites_[p][revealLine_][pos].setCard(c);
            cardSprites_[p][revealLine_][pos].setFaceUp(true);
        }
    }
    lineRevealed_[revealLine_] = true;

    // 计算该道赢家(纯展示,不结算)
    if (room) {
        int winners[MAX_PLAYERS];
        int cnt = Round::findWinners(room->players, pc, revealLine_, winners);
        char buf[96];
        if (cnt == 1) {
            int w = winners[0];
            std::vector<Card> three = {
                room->players[w].lines[revealLine_][0],
                room->players[w].lines[revealLine_][1],
                room->players[w].lines[revealLine_][2]
            };
            HandResult r = HandEvaluator::evaluate(three);
            const char* dn = (w == 0) ? "你" : room->players[w].name.c_str();
            std::snprintf(buf, sizeof(buf), "%s比牌: %s 赢 (%s)",
                          LINE_NAMES[revealLine_], dn, r.name().c_str());
        } else {
            std::snprintf(buf, sizeof(buf), "%s比牌: %d 家打平!",
                          LINE_NAMES[revealLine_], cnt);
        }
        info_.setText(buf);
    }
}

void SceneBattle::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (showNext_) btnNext_.handleEvent(e, win);
}

void SceneBattle::update(float dt) {
    if (!allRevealed()) {
        animTimer_ += dt;
        if (animTimer_ >= 1.0f) {   // 每 1 秒揭示一道
            animTimer_ = 0.f;
            revealNext();
        }
    }
}

void SceneBattle::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);

    // 四家牌面:已揭示的道显示正面,未揭示显示牌背
    Room* room = mgr_->room.get();
    const int pc = room ? room->playerCount : 0;
    for (int p = 0; p < 4 && p < pc; p++) {
        nameTags_[p].draw(win);
        chipTags_[p].draw(win);
        for (int line = 0; line < 3; line++) {
            for (int pos = 0; pos < 3; pos++) {
                cardSprites_[p][line][pos].draw(win);
            }
        }
    }
    info_.draw(win);
    if (showNext_) btnNext_.draw(win);
}
