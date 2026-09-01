#include "Player.h"

// ====== 构造函数 ======

Player::Player() : Player("玩家", false) {}

Player::Player(const std::string& playerName, bool ai)
    : name(playerName), isAI(ai) {
    clearRound();
}

// ====== 发牌：把 9 张牌放进 hand ======

void Player::setHand(const Card* cards, int count) {
    for (int i = 0; i < count && i < CARDS_PER_HAND; i++) {
        hand[i] = cards[i];
    }
}

// ====== 摆牌：按 order 数组把 9 张手牌摆成 3 道 ======
// order 的长度是 9：
//   order[0..2]  → 头道（lines[0]）
//   order[3..5]  → 中道（lines[1]）
//   order[6..8]  → 尾道（lines[2]）

void Player::arrangeByOrder(const int* order) {
    for (int line = 0; line < LINES_PER_HAND; line++) {
        for (int pos = 0; pos < CARDS_PER_LINE; pos++) {
            int handIdx = order[line * CARDS_PER_LINE + pos];
            lines[line][pos] = hand[handIdx];
        }
    }
    hasArranged = true;
}

// ====== 摆牌：单独放一张牌 ======

void Player::putCard(int handIndex, int lineId, int pos) {
    if (lineId >= 0 && lineId < LINES_PER_HAND
        && pos >= 0 && pos < CARDS_PER_LINE
        && handIndex >= 0 && handIndex < CARDS_PER_HAND) {
        lines[lineId][pos] = hand[handIndex];
    }
}

// ====== 清空本局数据 ======

void Player::clearRound() {
    hasArranged = false;
    for (int line = 0; line < LINES_PER_HAND; line++) {
        for (int pos = 0; pos < CARDS_PER_LINE; pos++) {
            lines[line][pos] = Card();  // 默认牌（小王）
        }
    }
}

// ====== 拿某一道的 3 张牌 ======

const Card* Player::getLine(int lineId) const {
    return lines[lineId];
}
