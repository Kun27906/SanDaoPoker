#ifndef ROOM_H
#define ROOM_H

#include "core/Player.h"
#include "core/Deck.h"
#include "core/Round.h"
#include "core/RuleConfig.h"
#include <string>

// 一桌游戏 = 一个房间: 管理玩家 牌堆 开新局与总账
class Room {
public:
    Player players[MAX_PLAYERS];
    int playerCount = 0;
    Deck deck;
    RoomConfig config;
    int pools[3] = {0, 0, 0};
    int currentRound = 0;

    int roundStartChips[MAX_PLAYERS] = {0};
    int roundHistory[32][MAX_PLAYERS] = {{0}};
    int historyCount = 0;

    Room();

    // configIndex 0~15, 换房清空玩家与筹码
    bool setRoomConfig(int configIndex);

    bool addPlayer(const std::string& name, bool isAI);

    bool startNewRound();

    std::string settleRound();

    bool isFinished() const;

    std::string getRanking() const;
};

#endif
