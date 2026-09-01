#ifndef ROUND_H
#define ROUND_H

#include "Player.h"
#include "Deck.h"
#include "HandEvaluator.h"
#include "RuleConfig.h"
#include <string>

// ====== 一局游戏 ======
// 职责：
//   1. 发牌（每人 9 张）
//   2. 收底注（三小池：头道池 / 中道池 / 尾道池）
//   3. 所有玩家交牌锁定后，三道轮次比牌
//   4. 结算：每道赢家拿走对应池，平局平分
//
// 规则说明（按分工方案）：
//   底注三小池：每人每局下 3 份底注（每道 1 份），分别放进三个池
//   交牌锁定：摆好牌后不能再改，所有人都交了才能比牌
//   三道轮次比牌：先比头道，再中道，最后尾道
//   平分赔付：某道并列最大 → 该池筹码平分给并列者（除不尽的余数作废）

class Round {
public:
    // 发牌：给 players（共 playerCount 人）每人发 9 张
    static void deal(Player* players, int playerCount, Deck& deck);

    // 收底注：每人扣 3 × ante 筹码，分到三个池（pools[0]头道 [1]中道 [2]尾道）
    static void collectAnte(Player* players, int playerCount, int ante, int pools[3]);

    // 交牌锁定检查：所有人都摆好牌了吗？
    static bool allArranged(const Player* players, int playerCount);

    // 找出某一道（lineId）并列最大的人，下标放进 winners
    // 返回并列人数（1 = 唯一赢家）
    static int findWinners(const Player* players, int playerCount, int lineId, int* winners);

    // 比牌结算：按头→中→尾逐道比，赢家拿池 / 平局平分，返回结果文字
    static std::string settle(Player* players, int playerCount, int pools[3]);

    // 比较某一道，返回唯一赢家的下标；平局返回 -1（一般用 findWinners）
    static int compareLine(const Player* players, int playerCount, int lineId);
};

#endif // ROUND_H
