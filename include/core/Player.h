#ifndef PLAYER_H
#define PLAYER_H

#include "core/Card.h"
#include "core/RuleConfig.h"
#include <string>

// 玩家: 手牌 三道 筹码
class Player {
public:
    std::string name;
    bool isAI = false;
    Card hand[CARDS_PER_HAND];
    Card lines[LINES_PER_HAND][CARDS_PER_LINE];
    int chips = START_CHIPS;
    int score = 0;
    bool hasArranged = false;

    Player();
    Player(const std::string& playerName, bool ai);

    void setHand(const Card* cards, int count);

    // order[0..2] 头道 [3..5] 中道 [6..8] 尾道, 值为 hand 下标且不重复
    void arrangeByOrder(const int* order);

    void putCard(int handIndex, int lineId, int pos);

    void clearRound();

    const Card* getLine(int lineId) const;
};

#endif
