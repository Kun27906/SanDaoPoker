#include "core/Room.h"
#include <algorithm>
#include <sstream>

Room::Room() {
    config = ROOM_CONFIGS[7];  // { "4人·休闲房", 4, 2, 3 }
}

// 换房 = 清空玩家 + 筹码重置 + 轮次清零，重新开始

bool Room::setRoomConfig(int configIndex) {
    if (configIndex < 0 || configIndex >= ROOM_CONFIG_COUNT) {
        return false;  // 下标越界
    }
    config = ROOM_CONFIGS[configIndex];
    playerCount = 0;          // 人清空，重新加人
    currentRound = 0;         // 局数清零
    historyCount = 0;
    pools[0] = pools[1] = pools[2] = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i] = Player();  // 重建玩家
    }
    return true;
}

bool Room::addPlayer(const std::string& name, bool isAI) {
    // 人数不能超过房间配置的人数
    if (playerCount >= config.players) {
        return false;
    }
    players[playerCount] = Player(name, isAI);
    playerCount++;
    return true;
}

// 只有人数凑齐才发牌，然后收底注

bool Room::startNewRound() {
    if (playerCount != config.players) {
        return false;  // 人数没满，不能开局
    }
    currentRound++;                             // 局数 +1
    // 快照本局开始前筹码 = 本局盈亏的基准线
    for (int i = 0; i < playerCount; i++) {
        roundStartChips[i] = players[i].chips;
    }
    Round::deal(players, playerCount, deck);    // 洗牌 + 发牌
    Round::collectAnte(players, playerCount, config.ante, pools);  // 收底注，进三小池
    return true;
}

// 交牌锁定：所有人都摆好牌才能比

std::string Room::settleRound() {
    if (!Round::allArranged(players, playerCount)) {
        return "还有人没交牌，不能比牌！请先摆好牌。\n";
    }
    std::string text = Round::settle(players, playerCount, pools);
    if (historyCount < 32) {
        for (int i = 0; i < playerCount; i++) {
            roundHistory[historyCount][i] = players[i].chips - roundStartChips[i];
        }
        historyCount++;
    }
    return text;
}

bool Room::isFinished() const {
    return currentRound >= config.rounds;
}

std::string Room::getRanking() const {
    std::stringstream out;

    // 复制一份下标，排序用
    int order[MAX_PLAYERS];
    for (int i = 0; i < playerCount; i++) {
        order[i] = i;
    }

    // 冒泡排序：按筹码从高到低
    for (int i = 0; i < playerCount; i++) {
        for (int j = 0; j < playerCount - 1 - i; j++) {
            if (players[order[j]].chips < players[order[j + 1]].chips) {
                std::swap(order[j], order[j + 1]);
            }
        }
    }

    // 输出排名
    out << "===== 总账（按筹码） =====" << std::endl;
    for (int i = 0; i < playerCount; i++) {
        int p = order[i];
        out << "第 " << (i + 1) << " 名：" << players[p].name
            << "（" << players[p].chips << " 筹码）" << std::endl;
    }
    return out.str();
}
