#ifndef HAND_EVALUATOR_H
#define HAND_EVALUATOR_H

#include "core/Card.h"
#include <string>
#include <vector>

// 牌型由小到大
enum class HandType {
    HighCard = 0,
    Pair,
    Straight,
    Flush,
    StraightFlush,
    ThreeOfAKind
};

struct HandResult {
    HandType type = HandType::HighCard;
    int keys[3] = {0, 0, 0};   // 比大小用点数, 由大到小
    bool is235 = false;        // 异花 2 3 5
    std::string name() const;
};

class HandEvaluator {
public:
    // 王按最有利的牌凑最大牌型
    static HandResult evaluate(const std::vector<Card>& three);

    // a 大返回 1, b 大返回 -1, 平返回 0
    static int compare(const HandResult& a, const HandResult& b);

private:
    static HandResult evaluateNormal(const Card& c1, const Card& c2, const Card& c3);

    static void enumerateJokers(std::vector<Card>& cards, int idx, HandResult& best);
};

#endif
