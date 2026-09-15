#ifndef DECK_H
#define DECK_H

#include "core/Card.h"
#include <vector>

// 一副牌 = 54张
// 职责：生成全副牌、洗牌、发牌

class Deck {
public:
    // 构造函数：生成一副完整的54张牌
    Deck();

    // 洗牌
    // seed 可选：传入则用固定种子，不传则用随机种子
    void shuffle(unsigned seed = 0);

    // 发牌：给 numPlayers 个人每人发 9 张，逆时针
    // 返回：二维数组，[玩家编号][9张牌]
    std::vector<std::vector<Card>> deal(int numPlayers) const;

    // 获取牌堆张数
    int size() const;

    // 重置为一副新牌
    void reset();

private:
    std::vector<Card> cards_;
};

#endif // DECK_H
