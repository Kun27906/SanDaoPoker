#ifndef RULE_CONFIG_H
#define RULE_CONFIG_H

// 规则数字集中在此, 改规则只改这一个文件
constexpr int MIN_PLAYERS = 2;
constexpr int MAX_PLAYERS = 6;

constexpr int CARDS_PER_HAND = 9;

constexpr int LINES_PER_HAND = 3;
constexpr int CARDS_PER_LINE = 3;

constexpr int START_CHIPS = 100;

// 一种房间 = 人数 + 底注 + 轮次
struct RoomConfig {
    const char* name;
    int players;
    int ante;
    int rounds;
};

constexpr int ROOM_CONFIG_COUNT = 22;

constexpr RoomConfig ROOM_CONFIGS[ROOM_CONFIG_COUNT] = {
    { "2人·注100·1局", 2, 100,  1 },
    { "2人·注100·2局", 2, 100,  2 },
    { "2人·注500·4局", 2, 500,  4 },
    { "3人·注100·4局", 3, 100,  4 },
    { "3人·注100·6局", 3, 100,  6 },
    { "3人·注500·6局", 3, 500,  6 },
    { "3人·注500·8局", 3, 500,  8 },
    { "4人·注500·6局", 4, 500,  6 },
    { "4人·注1000·4局", 4, 1000, 4 },
    { "4人·注1000·6局", 4, 1000, 6 },
    { "4人·注2000·6局", 4, 2000, 6 },
    { "4人·注2000·8局", 4, 2000, 8 },
    { "5人·注1000·8局", 5, 1000, 8 },
    { "5人·注2000·8局", 5, 2000, 8 },
    { "5人·注3000·4局", 5, 3000, 4 },
    { "5人·注3000·6局", 5, 3000, 6 },
    { "5人·注3000·8局", 5, 3000, 8 },
    { "6人·注3000·6局", 6, 3000, 6 },
    { "6人·注3000·8局", 6, 3000, 8 },
    { "6人·注5000·8局", 6, 5000, 8 },
    { "6人·注5000·16局", 6, 5000, 16 },
    { "6人·注10000·16局", 6, 10000, 16 }
};

// 罚金 = 阶梯倍数 x 本场底注, 只统计已结算完的局
inline int escapePenaltyFor(int playedRounds, int ante) {
    if (playedRounds <= 0) return 0;
    int mult;
    if (playedRounds <= 2)      mult = 1;
    else if (playedRounds <= 4) mult = 2;
    else if (playedRounds <= 8) mult = 3;
    else                        mult = 4;
    return mult * ante;
}

#endif
