#ifndef RULE_CONFIG_H
#define RULE_CONFIG_H

// ====== 游戏规则常量 ======
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

// ====== 房间配置 ======
// 一种房间 = 人数 + 底注 + 轮次
// 底注：每道 1 份，三小池 = 每局每人下 3 份底注
struct RoomConfig {
    const char* name;   // 房间名字（给界面显示用）
    int players;        // 人数（2~6）
    int ante;           // 底注（筹码数，每道 1 份）
    int rounds;         // 总轮次（打几局比总筹码）
};

// ====== 16 种预置房间 ======
// 覆盖 2~6 人 × 低/中/高注金，加 2 个特殊大赛房
constexpr int ROOM_CONFIG_COUNT = 16;

constexpr RoomConfig ROOM_CONFIGS[ROOM_CONFIG_COUNT] = {
    { "2人·练习房",   2,  1,  1 },   // 2人 小注 1局
    { "2人·休闲房",   2,  2,  3 },   // 2人 中注 3局
    { "2人·豪客房",   2,  5,  5 },   // 2人 高注 5局
    { "3人·练习房",   3,  1,  1 },   // 3人 小注 1局
    { "3人·休闲房",   3,  2,  3 },   // 3人 中注 3局
    { "3人·豪客房",   3,  5,  5 },   // 3人 高注 5局
    { "4人·练习房",   4,  1,  1 },   // 4人 小注 1局
    { "4人·休闲房",   4,  2,  3 },   // 4人 中注 3局（默认房）
    { "4人·豪客房",   4,  5,  5 },   // 4人 高注 5局
    { "5人·休闲房",   5,  2,  3 },   // 5人 中注 3局
    { "5人·豪客房",   5,  5,  5 },   // 5人 高注 5局
    { "6人·练习房",   6,  1,  1 },   // 6人 小注 1局
    { "6人·休闲房",   6,  2,  3 },   // 6人 中注 3局
    { "6人·豪客房",   6,  5,  5 },   // 6人 高注 5局
    { "4人·标准赛",   4,  3,  5 },   // 大赛：4人 中高注 5局
    { "4人·锦标赛",   4, 10,  5 }    // 大赛：4人 高注 5局
};

#endif // RULE_CONFIG_H
