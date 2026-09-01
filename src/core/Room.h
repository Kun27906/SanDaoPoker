#ifndef ROOM_H
#define ROOM_H

#include "Player.h"
#include "Deck.h"
#include "Round.h"
#include "RuleConfig.h"
#include <string>

// ====== 房间 ======
// 一桌游戏 = 一个房间
// 职责：
//   1. 选房间（16 种配置：人数 + 底注 + 轮次）
//   2. 管理玩家（人数要和房间配置一致）
//   3. 管理牌堆（Deck）
//   4. 开新局（发牌 + 收底注）、结算、看总账
//
// 用固定数组存玩家（最多 MAX_PLAYERS 个），不用 vector，简单好懂

class Room {
public:
    Player players[MAX_PLAYERS];  // 玩家列表（最多 6 个）
    int playerCount = 0;          // 当前有几个人
    Deck deck;                    // 牌堆
    RoomConfig config;            // 当前房间配置（人数/底注/轮次）
    int pools[3] = {0, 0, 0};     // 三个小池（头道/中道/尾道）
    int currentRound = 0;         // 第几局（从 1 开始）

    // 构造函数：默认选 "4人·休闲房"（底注2、3轮）
    Room();

    // 选房间：configIndex 是 0~15（ROOM_CONFIGS 的下标）
    // 换房会清空玩家和筹码，重新开始
    bool setRoomConfig(int configIndex);

    // 加一个玩家，成功返回 true；人满了返回 false
    bool addPlayer(const std::string& name, bool isAI);

    // 开始新一局：人数满员才发牌 + 收底注，返回是否成功
    bool startNewRound();

    // 结算本局：先检查所有人都交牌，再比牌；返回结果文字
    std::string settleRound();

    // 轮次打完了吗？
    bool isFinished() const;

    // 看总账（按筹码从高到低排），返回排名文字
    std::string getRanking() const;

    // 重开一桌：筹码全部重置，轮次清零（玩家保留）
    void resetGame();
};

#endif // ROOM_H
