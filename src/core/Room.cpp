#include "core/Room.h"
#include <algorithm>
#include <sstream>

Room::Room() {
    config = ROOM_CONFIGS[7];
}

// 换房清空玩家与筹码
bool Room::setRoomConfig(int configIndex) {
    if (configIndex < 0 || configIndex >= ROOM_CONFIG_COUNT) {
        return false;
    }
    config = ROOM_CONFIGS[configIndex];
    playerCount = 0;
    currentRound = 0;
    historyCount = 0;
    pools[0] = pools[1] = pools[2] = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i] = Player();
    }
    return true;
}

bool Room::addPlayer(const std::string& name, bool isAI) {
    if (playerCount >= config.players) {
        return false;
    }
    players[playerCount] = Player(name, isAI);
    playerCount++;
    return true;
}

// 人数满才发牌并收底注
bool Room::startNewRound() {
    if (playerCount != config.players) {
        return false;
    }
    currentRound++;
    // 开局筹码快照作为本局盈亏基准
    for (int i = 0; i < playerCount; i++) {
        roundStartChips[i] = players[i].chips;
    }
    Round::deal(players, playerCount, deck);
    Round::collectAnte(players, playerCount, config.ante, pools);
    return true;
}

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

    int order[MAX_PLAYERS];
    for (int i = 0; i < playerCount; i++) {
        order[i] = i;
    }

    // 按筹码从高到低
    for (int i = 0; i < playerCount; i++) {
        for (int j = 0; j < playerCount - 1 - i; j++) {
            if (players[order[j]].chips < players[order[j + 1]].chips) {
                std::swap(order[j], order[j + 1]);
            }
        }
    }

    out << "===== 总账（按筹码） =====" << std::endl;
    for (int i = 0; i < playerCount; i++) {
        int p = order[i];
        out << "第 " << (i + 1) << " 名：" << players[p].name
            << "（" << players[p].chips << " 筹码）" << std::endl;
    }
    return out.str();
}
