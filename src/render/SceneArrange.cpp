#include "render/SceneArrange.h"
#include "render/AssetManager.h"
#include "core/Room.h"
#include <algorithm>
#include <random>
#include <cstdio>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;

// 布局常量
constexpr float SLOT_X[3] = {500.f, 630.f, 760.f};   // 槽 x(每道3槽)
constexpr float LINE_Y[3] = {150.f, 310.f, 470.f};   // 三道 y
constexpr float HAND_X[9] = {90.f, 210.f, 330.f, 450.f, 570.f,
                             690.f, 810.f, 930.f, 1050.f};  // 手牌 x
constexpr float HAND_Y = 640.f;
constexpr float CARD_SCALE = 0.5f;   // 200x280 -> 100x140
constexpr float COUNTDOWN_SECONDS = 25.f;  // 组牌限时(常量,后期可调)

sf::RectangleShape makeSlot(const sf::Vector2f& pos, const sf::Vector2f& size) {
    sf::RectangleShape r(size);
    r.setPosition(pos);
    r.setFillColor(sf::Color(20, 26, 20, 200));
    r.setOutlineColor(sf::Color(140, 160, 140));
    r.setOutlineThickness(2.f);
    return r;
}
}

SceneArrange::SceneArrange(SceneManager* mgr) : mgr_(mgr) {
    // 防御:房间不存在则创建默认 4 人房
    if (!mgr_->room) {
        mgr_->room = std::make_unique<Room>();
        mgr_->room->setRoomConfig(7);
        mgr_->room->addPlayer("你", false);
        for (int i = 1; i < mgr_->room->config.players; i++) {
            char name[16];
            std::snprintf(name, sizeof(name), "AI-%d", i);
            mgr_->room->addPlayer(name, true);
        }
    }
    // 开局:发牌 + 收底注
    mgr_->room->startNewRound();

    // 背景
    if (const sf::Texture* bg = AssetManager::instance().background()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

    // 标题:房间名 + 第几局
    char title[64];
    std::snprintf(title, sizeof(title), "%s - 第%d局(底注%d)",
                  mgr_->room->config.name, mgr_->room->currentRound,
                  mgr_->room->config.ante);
    title_.setText(title);
    title_.setCharacterSize(30);
    title_.setColor(sf::Color::White);
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 36.f));

    hint_.setText("点击手牌放入当前道(左侧选道), 全部放完点[交牌]");
    hint_.setCharacterSize(18);
    hint_.setColor(sf::Color(210, 210, 210));
    hint_.centerOrigin();
    hint_.setPosition(sf::Vector2f(WW / 2.f, 78.f));

    // 手牌:9 张(scale 0.5)
    const Card* hand = mgr_->room->players[0].hand;
    for (int i = 0; i < 9; i++) {
        handSprites_[i].setCard(hand[i]);
        handSprites_[i].setFaceUp(true);
        handSprites_[i].setScale(CARD_SCALE);
        handSprites_[i].setPosition(sf::Vector2f(HAND_X[i], HAND_Y));
        handSlotRects_[i] = makeSlot(sf::Vector2f(HAND_X[i], HAND_Y),
                                     sf::Vector2f(100.f, 140.f));
    }

    // 三道槽
    for (int line = 0; line < 3; line++) {
        for (int pos = 0; pos < 3; pos++) {
            sf::Vector2f p(SLOT_X[pos], LINE_Y[line]);
            lineSprites_[line][pos].setScale(CARD_SCALE);
            lineSprites_[line][pos].setPosition(p);
            lineSlotRects_[line][pos] = makeSlot(p, sf::Vector2f(100.f, 140.f));
        }
    }

    // 道选择按钮
    const char* lineNames[3] = {"头道", "中道", "尾道"};
    for (int i = 0; i < 3; i++) {
        lineBtns_[i].setText(lineNames[i]);
        lineBtns_[i].setPosition(sf::Vector2f(30.f, LINE_Y[i]));
        lineBtns_[i].setSize(sf::Vector2f(110.f, 44.f));
        lineBtns_[i].setCallback([this, i]() {
            currentLine_ = i;
            // 刷新高亮
            for (int j = 0; j < 3; j++) {
                if (j == currentLine_) {
                    lineBtns_[j].setColors(sf::Color(46, 160, 80), sf::Color(70, 190, 100), sf::Color(30, 120, 55));
                } else {
                    lineBtns_[j].setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
                }
            }
        });
    }
    lineBtns_[0].setColors(sf::Color(46, 160, 80), sf::Color(70, 190, 100), sf::Color(30, 120, 55));  // 默认选中头道

    // 一键重置 / 交牌
    btnReset_.setText("一键重置");
    btnReset_.setPosition(sf::Vector2f(1140.f, 150.f));
    btnReset_.setSize(sf::Vector2f(110.f, 50.f));
    btnReset_.setCallback([this]() { resetArrange(); });

    btnSubmit_.setText("交牌");
    btnSubmit_.setPosition(sf::Vector2f(1140.f, 220.f));
    btnSubmit_.setSize(sf::Vector2f(110.f, 50.f));
    btnSubmit_.setCallback([this]() { submit(); });

    // 倒计时
    countdown_ = CountdownBar(COUNTDOWN_SECONDS, sf::Vector2f(390.f, 100.f), sf::Vector2f(500.f, 28.f));
    countdown_.start();
}

void SceneArrange::placeCard(int handIdx) {
    if (handUsed_[handIdx] || submitted_) return;
    Player& me = mgr_->room->players[0];
    for (int pos = 0; pos < 3; pos++) {
        if (!slotUsed_[currentLine_][pos]) {
            me.putCard(handIdx, currentLine_, pos);
            slotUsed_[currentLine_][pos] = true;
            handUsed_[handIdx] = true;
            lineSprites_[currentLine_][pos].setCard(me.hand[handIdx]);
            lineSprites_[currentLine_][pos].setFaceUp(true);
            hint_.setText("继续放牌, 或点[重置]重摆");
            break;
        }
    }
}

void SceneArrange::resetArrange() {
    if (submitted_) return;
    mgr_->room->players[0].clearRound();  // 清空 lines + hasArranged
    handUsed_.fill(false);
    for (auto& row : slotUsed_) row.fill(false);
    // 方案1:一键重置不重置倒计时(倒计时持续走,超时仍会自动交牌,杜绝无限重置)
    hint_.setText("已一键重置, 请重新分三道(倒计时继续)");
}

bool SceneArrange::allPlaced() const {
    for (bool b : handUsed_) {
        if (!b) return false;
    }
    return true;
}

void SceneArrange::submit() {
    if (submitted_) return;
    if (!allPlaced()) {
        hint_.setText("还有牌没摆完! 请把 9 张牌全部分到三道");
        return;
    }
    submitted_ = true;
    Room* room = mgr_->room.get();

    // 真人交牌锁定
    room->players[0].hasArranged = true;

    // AI 玩家:假AI随机组牌(阶段8替换为 AIPlayer::decideOrderStyled)
    std::mt19937 rng(std::random_device{}());
    for (int p = 1; p < room->playerCount; p++) {
        int order[9];
        for (int i = 0; i < 9; i++) order[i] = i;
        std::shuffle(order, order + 9, rng);
        room->players[p].arrangeByOrder(order);
        room->players[p].hasArranged = true;
    }

    mgr_->changeTo(SceneId::Battle);
}

void SceneArrange::autoSubmit() {
    if (submitted_) return;
    // 超时:把剩余手牌按顺序填入所有空槽
    int hi = 0;
    for (int line = 0; line < 3 && hi < 9; line++) {
        for (int pos = 0; pos < 3 && hi < 9; pos++) {
            if (slotUsed_[line][pos]) continue;
            while (hi < 9 && handUsed_[hi]) hi++;
            if (hi >= 9) break;
            mgr_->room->players[0].putCard(hi, line, pos);
            slotUsed_[line][pos] = true;
            handUsed_[hi] = true;
            lineSprites_[line][pos].setCard(mgr_->room->players[0].hand[hi]);
            lineSprites_[line][pos].setFaceUp(true);
        }
    }
    hint_.setText("时间到, 自动摆牌并交牌");
    submit();
}

void SceneArrange::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    for (auto& b : lineBtns_) b.handleEvent(e, win);
    btnReset_.handleEvent(e, win);
    btnSubmit_.handleEvent(e, win);

    // 点击手牌 -> 放入当前道
    if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mpos = win.mapPixelToCoords(sf::Mouse::getPosition(win));
        for (int i = 0; i < 9; i++) {
            if (!handUsed_[i] && handSprites_[i].getBounds().contains(mpos)) {
                placeCard(i);
                break;
            }
        }
    }
}

void SceneArrange::update(float dt) {
    countdown_.update(dt);
    // 超时自动交牌(只触发一次;实例级标志,场景重建后重置)
    if (countdown_.isFinished() && !timeoutFired_) {
        timeoutFired_ = true;
        autoSubmit();
    }
}

void SceneArrange::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    hint_.draw(win);

    // 三道槽:空槽画框, 已摆画牌
    for (int line = 0; line < 3; line++) {
        for (int pos = 0; pos < 3; pos++) {
            if (slotUsed_[line][pos]) {
                lineSprites_[line][pos].draw(win);
            } else {
                win.draw(lineSlotRects_[line][pos]);
            }
        }
    }

    // 手牌区:未用画牌, 已用画空框
    for (int i = 0; i < 9; i++) {
        if (handUsed_[i]) {
            win.draw(handSlotRects_[i]);
        } else {
            handSprites_[i].draw(win);
        }
    }

    for (auto& b : lineBtns_) b.draw(win);
    btnReset_.draw(win);
    btnSubmit_.draw(win);
    countdown_.draw(win);
}
