#include "render/SceneArrange.h"
#include "render/AssetManager.h"
#include "render/Account.h"
#include "render/SoundManager.h"
#include "core/NameGen.h"
#include "ai/AIPlayer.h"
#include "core/Room.h"
#include <algorithm>
#include <random>
#include <cstdio>
#include <cmath>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;

// 布局常量
constexpr float SLOT_X[3] = {500.f, 630.f, 760.f};   // 槽 x(每道3槽)
constexpr float LINE_Y[3] = {150.f, 310.f, 470.f};   // 三道 y
constexpr float HAND_X[9] = {90.f, 210.f, 330.f, 450.f, 570.f,
                             690.f, 810.f, 930.f, 1050.f};  // 手牌 x
constexpr float HAND_Y = 640.f;
constexpr float CARD_SCALE = 0.5f;            // 200x280 -> 100x140
constexpr float CARDW = 200.f * CARD_SCALE;   // 牌宽 100
constexpr float CARDH = 280.f * CARD_SCALE;   // 牌高 140 (避开 core/Card.h 的保护宏名冲突)
constexpr float COUNTDOWN_SECONDS = 40.f;     // 组牌限时(延长至 40 秒)

// 拖拽参数
constexpr float SNAP_DIST = 62.f;    // 吸附: 牌中心距槽中心 < 该值则吸附到槽
constexpr float DROP_DIST = 88.f;    // 落位判定距离
constexpr float DRAG_THRESH = 6.f;   // 位移超过该值才算拖拽(否则算单击)
constexpr float FLY_DUR = 0.26f;     // 飞回动画时长(秒)

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
        std::string used[MAX_PLAYERS];
        int uc = 0;
        std::string meName = Account::instance().ensureNickname();
        used[uc++] = meName;
        mgr_->room->addPlayer(meName, false);
        for (int i = 1; i < mgr_->room->config.players; i++) {
            std::string nm = makeUniqueNickname(used, uc);
            used[uc++] = nm;
            mgr_->room->addPlayer(nm, true);
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

    // 每局开局:随机掷一种牌背颜色
    AssetManager::instance().rollBack();

    // 标题
    char title[64];
    std::snprintf(title, sizeof(title), "第 %d 局 · 底注 %d",
                  mgr_->room->currentRound, mgr_->room->config.ante);
    title_.setText(title);
    title_.setCharacterSize(32);
    title_.setColor(sf::Color(255, 215, 0));
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 36.f));

    hint_.setText("拖动牌到三道槽位(靠近自动吸附); 单击已放置的牌可收回; 全部放完点[交牌]");
    hint_.setCharacterSize(18);
    hint_.setColor(sf::Color(210, 210, 210));
    hint_.centerOrigin();
    hint_.setPosition(sf::Vector2f(WW / 2.f, 78.f));

    // 手牌:9 张
    const Card* hand = mgr_->room->players[0].hand;
    for (int i = 0; i < 9; i++) {
        handSprites_[i].setCard(hand[i]);
        handSprites_[i].setFaceUp(true);
        handSprites_[i].setScale(CARD_SCALE);
        handSprites_[i].setPosition(sf::Vector2f(HAND_X[i], HAND_Y));
        handSlotRects_[i] = makeSlot(sf::Vector2f(HAND_X[i], HAND_Y),
                                     sf::Vector2f(CARDW, CARDH));
    }

    // 三道槽 + 模型初始化
    for (int line = 0; line < 3; line++) {
        slotHand_[line].fill(-1);
        for (int pos = 0; pos < 3; pos++) {
            sf::Vector2f p = slotPos(line, pos);
            lineSprites_[line][pos].setScale(CARD_SCALE);
            lineSprites_[line][pos].setPosition(p);
            lineSlotRects_[line][pos] = makeSlot(p, sf::Vector2f(CARDW, CARDH));
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
            for (int j = 0; j < 3; j++) {
                if (j == currentLine_) {
                    lineBtns_[j].setColors(sf::Color(46, 160, 80), sf::Color(70, 190, 100), sf::Color(30, 120, 55));
                } else {
                    lineBtns_[j].setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
                }
            }
        });
    }
    lineBtns_[0].setColors(sf::Color(46, 160, 80), sf::Color(70, 190, 100), sf::Color(30, 120, 55));

    // 一键重置 / 交牌
    btnReset_.setText("一键重置");
    btnReset_.setPosition(sf::Vector2f(1140.f, 150.f));
    btnReset_.setSize(sf::Vector2f(110.f, 50.f));
    btnReset_.setCallback([this]() { resetArrange(); });

    btnSubmit_.setText("交牌");
    btnSubmit_.setPosition(sf::Vector2f(1140.f, 220.f));
    btnSubmit_.setSize(sf::Vector2f(110.f, 50.f));
    btnSubmit_.setCallback([this]() { submit(); });

    // 倒计时(40 秒)
    countdown_ = CountdownBar(COUNTDOWN_SECONDS, sf::Vector2f(390.f, 100.f), sf::Vector2f(500.f, 28.f));
    countdown_.start();

    chipBar_.setPosition(sf::Vector2f(1280.f - 250.f - 20.f, 16.f));
}

// ---- 槽位/几何辅助 ----
sf::Vector2f SceneArrange::slotPos(int line, int pos) const {
    return sf::Vector2f(SLOT_X[pos], LINE_Y[line]);
}

int SceneArrange::nearestSlot(const sf::Vector2f& cardTopLeft, float maxDist) const {
    sf::Vector2f c(cardTopLeft.x + CARDW / 2.f, cardTopLeft.y + CARDH / 2.f);
    int best = -1;
    float bestD = maxDist;
    for (int line = 0; line < 3; line++) {
        for (int pos = 0; pos < 3; pos++) {
            sf::Vector2f sc(SLOT_X[pos] + CARDW / 2.f, LINE_Y[line] + CARDH / 2.f);
            float dx = c.x - sc.x, dy = c.y - sc.y;
            float d = std::sqrt(dx * dx + dy * dy);
            if (d < bestD) { bestD = d; best = line * 3 + pos; }
        }
    }
    return best;
}

// ---- 模型 <-> 房间同步 ----
void SceneArrange::refreshSlotSprites() {
    const Card* hand = mgr_->room->players[0].hand;
    for (int line = 0; line < 3; line++) {
        for (int pos = 0; pos < 3; pos++) {
            int hi = slotHand_[line][pos];
            if (hi >= 0) {
                lineSprites_[line][pos].setCard(hand[hi]);
                lineSprites_[line][pos].setFaceUp(true);
                lineSprites_[line][pos].setScale(CARD_SCALE);
                lineSprites_[line][pos].setPosition(slotPos(line, pos));
            }
        }
    }
}

void SceneArrange::rebuildLines() {
    Player& me = mgr_->room->players[0];
    me.clearRound();   // 清空 lines(并把 hasArranged 置 false)
    for (int line = 0; line < 3; line++) {
        for (int pos = 0; pos < 3; pos++) {
            if (slotHand_[line][pos] >= 0) {
                me.putCard(slotHand_[line][pos], line, pos);
            }
        }
    }
}

// ---- 放置 / 收回 ----
void SceneArrange::placeAt(int handIdx, int line, int pos) {
    if (submitted_) return;
    if (line < 0 || line > 2 || pos < 0 || pos > 2) return;
    if (handIdx < 0 || handIdx > 8) return;
    if (slotHand_[line][pos] == handIdx) return;
    // 该手牌若已在其它槽, 先移除
    for (int l = 0; l < 3; l++)
        for (int p = 0; p < 3; p++)
            if (slotHand_[l][p] == handIdx) slotHand_[l][p] = -1;
    slotHand_[line][pos] = handIdx;
    handUsed_[handIdx] = true;
    refreshSlotSprites();
    rebuildLines();
    SoundManager::instance().playClick();   // 放入槽位:点击音效
    hint_.setText("已放入; 可继续拖拽, 或单击已放置的牌收回");
}

void SceneArrange::placeCard(int handIdx) {
    if (submitted_ || handUsed_[handIdx]) return;
    for (int pos = 0; pos < 3; pos++) {
        if (slotHand_[currentLine_][pos] < 0) {
            placeAt(handIdx, currentLine_, pos);
            return;
        }
    }
    hint_.setText("当前道已满, 请换道或拖到其它道");
}

void SceneArrange::startFlyBack(int handIdx) {
    flying_ = true;
    flyHand_ = handIdx;
    flyTo_ = sf::Vector2f(HAND_X[handIdx], HAND_Y);
    flyT_ = 0.f;
    flySprite_.setCard(mgr_->room->players[0].hand[handIdx]);
    flySprite_.setFaceUp(true);
    flySprite_.setScale(CARD_SCALE);
    flySprite_.setPosition(flyFrom_);
}

void SceneArrange::returnToHand(int line, int pos) {
    if (submitted_) return;
    int hi = slotHand_[line][pos];
    if (hi < 0) return;
    slotHand_[line][pos] = -1;
    flyFrom_ = slotPos(line, pos);
    startFlyBack(hi);                       // 牌"飞回"下方原位置
    SoundManager::instance().playClick();   // 收回:点击音效
    rebuildLines();
    hint_.setText("已收回手牌");
}

// ---- 拖拽 ----
void SceneArrange::beginDrag(int handIdx, int fromLine, int fromPos, const sf::Vector2f& mouse) {
    dragHand_ = handIdx;
    dragFromLine_ = fromLine;
    dragFromPos_ = fromPos;
    sf::Vector2f cardPos = (fromLine >= 0) ? slotPos(fromLine, fromPos)
                                           : sf::Vector2f(HAND_X[handIdx], HAND_Y);
    dragGrab_ = sf::Vector2f(mouse.x - cardPos.x, mouse.y - cardPos.y);
    dragPos_ = cardPos;
    dragSprite_.setCard(mgr_->room->players[0].hand[handIdx]);
    dragSprite_.setFaceUp(true);
    dragSprite_.setScale(CARD_SCALE);
    dragSprite_.setPosition(dragPos_);
    pendingDrag_ = true;
    dragging_ = false;
    snapSlot_ = -1;
}

void SceneArrange::dropDrag() {
    int t = nearestSlot(dragPos_, DROP_DIST);
    bool landed = false;
    if (dragFromLine_ >= 0) {
        // 来自槽位: 先取出
        slotHand_[dragFromLine_][dragFromPos_] = -1;
        if (t >= 0 && t != dragFromLine_ * 3 + dragFromPos_) {
            int tl = t / 3, tp = t % 3;
            int occ = slotHand_[tl][tp];
            if (occ >= 0) {
                // 目标已有牌: 交换(占位牌回到来源槽)
                slotHand_[dragFromLine_][dragFromPos_] = occ;
            }
            slotHand_[tl][tp] = dragHand_;
            landed = true;
        } else {
            // 落回原槽
            slotHand_[dragFromLine_][dragFromPos_] = dragHand_;
        }
    } else {
        // 来自手牌
        if (t >= 0) {
            int tl = t / 3, tp = t % 3;
            if (slotHand_[tl][tp] < 0) {
                slotHand_[tl][tp] = dragHand_;
                handUsed_[dragHand_] = true;
                landed = true;
            }
        }
    }
    refreshSlotSprites();
    rebuildLines();
    if (landed) SoundManager::instance().playClick();   // 放入槽位:点击音效
    dragging_ = false;
    pendingDrag_ = false;
    snapSlot_ = -1;
    dragHand_ = -1;
    dragFromLine_ = dragFromPos_ = -1;
}

// ---- 重置 / 交牌 ----
void SceneArrange::resetArrange() {
    if (submitted_) return;
    for (auto& row : slotHand_) row.fill(-1);
    handUsed_.fill(false);
    refreshSlotSprites();
    rebuildLines();
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
    rebuildLines();                        // 确保 lines 与界面一致
    room->players[0].hasArranged = true;   // 真人交牌锁定

    // AI 玩家:接入 AIPlayer 真实决策(难度/风格取自当前配置;胜率表 assets/ai/winrate.bin)
    for (int p = 1; p < room->playerCount; p++) {
        int order[9];
        AIPlayer::decideOrderAuto(room->players[p].hand, room->playerCount, order);
        room->players[p].arrangeByOrder(order);
        room->players[p].hasArranged = true;
    }
    mgr_->changeTo(SceneId::Battle);
}

void SceneArrange::autoSubmit() {
    if (submitted_) return;
    if (flying_) { flying_ = false; handUsed_[flyHand_] = false; }  // 结算在途动画
    // 超时:把剩余手牌按顺序填入所有空槽
    int hi = 0;
    for (int line = 0; line < 3; line++) {
        for (int pos = 0; pos < 3; pos++) {
            if (slotHand_[line][pos] >= 0) continue;
            while (hi < 9 && handUsed_[hi]) hi++;
            if (hi >= 9) break;
            slotHand_[line][pos] = hi;
            handUsed_[hi] = true;
            hi++;
        }
    }
    refreshSlotSprites();
    rebuildLines();
    hint_.setText("时间到, 自动摆牌并交牌");
    submit();
}

// ---- 事件 ----
void SceneArrange::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    for (auto& b : lineBtns_) b.handleEvent(e, win);
    btnReset_.handleEvent(e, win);
    btnSubmit_.handleEvent(e, win);

    if (submitted_) return;
    sf::Vector2f mpos = win.mapPixelToCoords(sf::Mouse::getPosition(win));

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        if (flying_) return;
        pressPos_ = mpos;
        // 先看手牌
        int hitHand = -1;
        for (int i = 0; i < 9; i++) {
            if (!handUsed_[i] && handSprites_[i].getBounds().contains(mpos)) { hitHand = i; break; }
        }
        if (hitHand >= 0) { beginDrag(hitHand, -1, -1, mpos); return; }
        // 再看已放置的牌
        for (int line = 0; line < 3; line++) {
            for (int pos = 0; pos < 3; pos++) {
                if (slotHand_[line][pos] < 0) continue;
                sf::FloatRect r(slotPos(line, pos), sf::Vector2f(CARDW, CARDH));
                if (r.contains(mpos)) { beginDrag(slotHand_[line][pos], line, pos, mpos); return; }
            }
        }
    } else if (e.type == sf::Event::MouseMoved) {
        if (pendingDrag_ && !dragging_) {
            float dx = mpos.x - pressPos_.x, dy = mpos.y - pressPos_.y;
            if (dx * dx + dy * dy > DRAG_THRESH * DRAG_THRESH) dragging_ = true;
        }
        if (dragging_) {
            dragPos_ = sf::Vector2f(mpos.x - dragGrab_.x, mpos.y - dragGrab_.y);
            int t = nearestSlot(dragPos_, SNAP_DIST);
            snapSlot_ = t;
            if (t >= 0) dragPos_ = slotPos(t / 3, t % 3);   // 吸附到槽
        }
    } else if (e.type == sf::Event::MouseButtonReleased && e.mouseButton.button == sf::Mouse::Left) {
        if (dragging_) {
            dropDrag();
        } else if (pendingDrag_) {
            if (dragFromLine_ >= 0) {
                returnToHand(dragFromLine_, dragFromPos_);   // 单击已放置牌 -> 收回
            } else if (dragHand_ >= 0) {
                placeCard(dragHand_);                        // 单击手牌 -> 放入当前道
            }
            pendingDrag_ = false;
            snapSlot_ = -1;
            dragHand_ = -1;
            dragFromLine_ = dragFromPos_ = -1;
        }
    }
}

void SceneArrange::update(float dt) {
    // 飞回动画
    if (flying_) {
        flyT_ += dt / FLY_DUR;
        if (flyT_ >= 1.f) {
            flyT_ = 1.f;
            flying_ = false;
            handUsed_[flyHand_] = false;   // 动画结束:牌正式回到手牌区
        } else {
            sf::Vector2f p(flyFrom_.x + (flyTo_.x - flyFrom_.x) * flyT_,
                           flyFrom_.y + (flyTo_.y - flyFrom_.y) * flyT_);
            flySprite_.setPosition(p);
        }
    }
    countdown_.update(dt);
    if (countdown_.isFinished() && !timeoutFired_) {
        timeoutFired_ = true;
        autoSubmit();
    }
}

void SceneArrange::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    hint_.draw(win);

    // 三道槽:已摆画牌(拖拽来源槽隐藏), 空槽画框, 吸附目标高亮
    for (int line = 0; line < 3; line++) {
        for (int pos = 0; pos < 3; pos++) {
            bool dragOrigin = dragging_ && dragFromLine_ == line && dragFromPos_ == pos;
            bool snapHere = dragging_ && snapSlot_ == line * 3 + pos;
            if (slotHand_[line][pos] >= 0 && !dragOrigin) {
                lineSprites_[line][pos].draw(win);
            } else {
                win.draw(lineSlotRects_[line][pos]);
                if (snapHere) {
                    sf::RectangleShape hl(sf::Vector2f(CARDW, CARDH));
                    hl.setPosition(slotPos(line, pos));
                    hl.setFillColor(sf::Color(255, 215, 0, 60));
                    hl.setOutlineColor(sf::Color(255, 215, 0));
                    hl.setOutlineThickness(3.f);
                    win.draw(hl);
                }
            }
        }
    }

    // 手牌区:未用画牌(正在拖动的隐藏), 已用/飞行中画空框
    for (int i = 0; i < 9; i++) {
        bool draggedFromHand = dragging_ && dragFromLine_ < 0 && dragHand_ == i;
        if (handUsed_[i] || draggedFromHand) {
            win.draw(handSlotRects_[i]);
        } else {
            handSprites_[i].draw(win);
        }
    }

    // 拖动中的牌(最上层; 位置随鼠标实时更新, 含吸附)
    if (dragging_) {
        dragSprite_.setPosition(dragPos_);
        dragSprite_.draw(win);
    }
    // 飞回中的牌
    if (flying_) flySprite_.draw(win);

    for (auto& b : lineBtns_) b.draw(win);
    btnReset_.draw(win);
    btnSubmit_.draw(win);
    countdown_.draw(win);
    chipBar_.draw(win, Account::instance().balance());
}
