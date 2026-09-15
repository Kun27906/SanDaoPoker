#ifndef ROUND_H
#define ROUND_H

#include "core/Player.h"
#include "core/Deck.h"
#include "core/HandEvaluator.h"
#include "core/RuleConfig.h"
#include <string>

// 发牌 收底注 逐道比牌 结算
class Round {
public:
    static void deal(Player* players, int playerCount, Deck& deck);

    static void collectAnte(Player* players, int playerCount, int ante, int pools[3]);

    static bool allArranged(const Player* players, int playerCount);

    // 返回并列人数, 下标写入 winners
    static int findWinners(const Player* players, int playerCount, int lineId, int* winners);

    // 头 中 尾逐道比, 赢家拿池平局平分
    static std::string settle(Player* players, int playerCount, int pools[3]);
};

#endif
