#ifndef RULE_CONFIG_H
#define RULE_CONFIG_H

// 所有规则数字都集中放在这里，改规则只改这一个文件

// 人数：一桌最少 2 人，最多 6 人
constexpr int MIN_PLAYERS = 2;
constexpr int MAX_PLAYERS = 6;

// 手牌：每人 9 张
constexpr int CARDS_PER_HAND = 9;

// 分道：9 张牌分成 3 道，每道 3 张
constexpr int LINES_PER_HAND = 3;
constexpr int CARDS_PER_LINE = 3;

// 每个玩家开局带的筹码
constexpr int START_CHIPS = 100;

// 一种房间 = 人数 + 底注 + 轮次
// 注金：每人每局下注金 1 份(总池=人数×注金, 均分三小池, 余数归尾道)
struct RoomConfig {
    const char* name;   // 房间名字（给界面显示用）
    int players;        // 人数（2~6）
    int ante;           // 注金（筹码数，每人每局下注 1 份）
    int rounds;         // 总轮次（打几局比总筹码）
};

// 房间 = 人数 × (原始注金, 轮次); 注金为每人每局下注额, 总池均分三小池
constexpr int ROOM_CONFIG_COUNT = 22;

constexpr RoomConfig ROOM_CONFIGS[ROOM_CONFIG_COUNT] = {
    // 二人房(3)
    { "2人·注100·1局", 2, 100,  1 },
    { "2人·注100·2局", 2, 100,  2 },
    { "2人·注500·4局", 2, 500,  4 },
    // 三人房(4)
    { "3人·注100·4局", 3, 100,  4 },
    { "3人·注100·6局", 3, 100,  6 },
    { "3人·注500·6局", 3, 500,  6 },
    { "3人·注500·8局", 3, 500,  8 },
    // 四人房(5)
    { "4人·注500·6局", 4, 500,  6 },   // 下标7(默认房)
    { "4人·注1000·4局", 4, 1000, 4 },
    { "4人·注1000·6局", 4, 1000, 6 },
    { "4人·注2000·6局", 4, 2000, 6 },
    { "4人·注2000·8局", 4, 2000, 8 },
    // 五人房(5)
    { "5人·注1000·8局", 5, 1000, 8 },
    { "5人·注2000·8局", 5, 2000, 8 },
    { "5人·注3000·4局", 5, 3000, 4 },
    { "5人·注3000·6局", 5, 3000, 6 },
    { "5人·注3000·8局", 5, 3000, 8 },
    // 六人房(5)
    { "6人·注3000·6局", 6, 3000, 6 },
    { "6人·注3000·8局", 6, 3000, 8 },
    { "6人·注5000·8局", 6, 5000, 8 },
    { "6人·注5000·16局", 6, 5000, 16 },
    { "6人·注10000·16局", 6, 10000, 16 }
};

// 与本场"已游玩局数"和"本场每局底注"挂钩:
//   0 局(一局都没打完) -> 不罚;  1-2 局 -> 罚 1 局底注;  3-4 局 -> 2 局;  5-8 局 -> 3 局;  9 局及以上 -> 4 局
// (本场最长 16 局, 可逃跑时点为本场第 1~15 局结算后; 9 局以上统一按 4 局底注封顶)
// playedRounds 只统计【已结算完成】的局 —— 局内(发牌/组牌/比牌)提前退出时当前这局不计入。
inline int escapePenaltyFor(int playedRounds, int ante) {
    if (playedRounds <= 0) return 0;   // 一局都没打完就退出: 不罚
    int mult;
    if (playedRounds <= 2)      mult = 1;
    else if (playedRounds <= 4) mult = 2;
    else if (playedRounds <= 8) mult = 3;
    else                        mult = 4;
    return mult * ante;
}

#endif // RULE_CONFIG_H
