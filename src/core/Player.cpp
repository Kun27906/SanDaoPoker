#include "core/Player.h"

Player::Player() : Player("玩家", false) {}

Player::Player(const std::string& playerName, bool ai)
    : name(playerName), isAI(ai) {
    clearRound();
}

void Player::setHand(const Card* cards, int count) {
    for (int i = 0; i < count && i < CARDS_PER_HAND; i++) {
        hand[i] = cards[i];
    }
}

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

void Player::putCard(int handIndex, int lineId, int pos) {
    if (lineId >= 0 && lineId < LINES_PER_HAND
        && pos >= 0 && pos < CARDS_PER_LINE
        && handIndex >= 0 && handIndex < CARDS_PER_HAND) {
        lines[lineId][pos] = hand[handIndex];
    }
}

void Player::clearRound() {
    hasArranged = false;
    for (int line = 0; line < LINES_PER_HAND; line++) {
        for (int pos = 0; pos < CARDS_PER_LINE; pos++) {
            lines[line][pos] = Card();  // 默认牌（小王）
        }
    }
}

const Card* Player::getLine(int lineId) const {
    return lines[lineId];
}
