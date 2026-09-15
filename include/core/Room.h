#ifndef ROOM_H
#define ROOM_H

#include "core/Player.h"
#include "core/Deck.h"
#include "core/Round.h"
#include "core/RuleConfig.h"
#include <string>

// 一桌游戏 = 一个房间
// 职责：
// 1. 选房间
// 2. 管理玩家
// 3. 管理牌堆
// 4. 开新局、结算、看总账
// 用固定数组存玩家，不用 vector，简单好懂

class Room {
public:
    Player players[MAX_PLAYERS];  // 玩家列表
    int playerCount = 0;  // 当前有几个人
    Deck deck;  // 牌堆
    RoomConfig config;  // 当前房间配置
    int pools[3] = {0, 0, 0};  // 三个小池
    int currentRound = 0;  // 第几局

    int roundStartChips[MAX_PLAYERS] = {0};  // 本局开始前筹码快照
    int roundHistory[32][MAX_PLAYERS] = {{0}};
    int historyCount = 0;  // 已结算局数

  // 构造函数：默认选 "4人·休闲房"
    Room();

  // 选房间：configIndex 是 0~15
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

  // 看总账，返回排名文字
    std::string getRanking() const;
};

#endif // ROOM_H
