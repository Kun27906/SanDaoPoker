#ifndef PLAYER_H
#define PLAYER_H

#include "Card.h"
#include "RuleConfig.h"
#include <string>

// ====== 玩家 ======
// 一个玩家有：名字、9 张手牌、摆好的 3 道牌、总分
// hand[9]     ：刚发到手的 9 张牌
// lines[3][3] ：摆好之后的牌，lines[0]=头道, lines[1]=中道, lines[2]=尾道

class Player {
public:
    std::string name;              // 名字
    bool isAI = false;             // 是不是电脑
    Card hand[CARDS_PER_HAND];     // 9 张手牌
    Card lines[LINES_PER_HAND][CARDS_PER_LINE];  // 摆好的 3 道牌
    int chips = START_CHIPS;       // 筹码（开局 100，下底注、赢池子都会变）
    int score = 0;                 // 赢道数（统计用，每赢一道 +1）
    bool hasArranged = false;      // 是否已经摆好牌（交牌锁定）

    // 构造函数：名字 + 是不是电脑
    Player();
    Player(const std::string& playerName, bool ai);

    // 发牌时调用：把 9 张牌放进 hand
    void setHand(const Card* cards, int count);

    // 摆牌：把 9 张手牌按给定顺序摆成 3 道
    // order 是一个 9 长度的数组，order[0..2] 是头道、order[3..5] 是中道、order[6..8] 是尾道
    // 每个数字是 hand 里的下标（0~8），不能重复
    void arrangeByOrder(const int* order);

    // 摆牌：直接指定每张牌放进哪一道（lineId 0~2，位置 pos 0~2）
    void putCard(int handIndex, int lineId, int pos);

    // 清空本局数据（开局时调用）
    void clearRound();

    // 拿某一道的 3 张牌（返回数组指针）
    const Card* getLine(int lineId) const;
};

#endif // PLAYER_H
