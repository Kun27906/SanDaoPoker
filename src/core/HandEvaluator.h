#ifndef HAND_EVALUATOR_H
#define HAND_EVALUATOR_H

#include "Card.h"
#include <string>
#include <vector>

// ====== 牌型等级（从弱到强） ======
// 扎金花的牌型，从小到大排列
enum class HandType {
    HighCard = 0,       // 散牌：啥都不是，比如 2, 7, K
    Pair,               // 对子：两张一样，比如 5, 5, 9
    Straight,           // 顺子：点数连起来，比如 4, 5, 6（不同花色）
    Flush,              // 金花：三张同花色，比如 3张都是黑桃
    StraightFlush,      // 同花顺：同花色又连续，比如 黑桃 7,8,9
    ThreeOfAKind        // 豹子：三张一样，比如 3张 K（最大！）
};

// ====== 一手牌（3张）的判断结果 ======
struct HandResult {
    HandType type = HandType::HighCard;  // 牌型
    int keys[3] = {0, 0, 0};             // 比大小用的点数，从大到小排
    bool is235 = false;                  // 是不是"异花 2 3 5"（能吃掉豹子的神牌）
    std::string name() const;            // 牌型的中文名字（"豹子"、"顺子"等）
};

// ====== 牌型判断器 ======
// 作用：给 3 张牌，告诉你是什么牌型；给两手牌，告诉你谁大
class HandEvaluator {
public:
    // 判断 3 张牌是什么牌型
    // 王会自动变成"最有利"的牌来凑最大牌型
    static HandResult evaluate(const std::vector<Card>& three);

    // 比较两手牌：a 大返回 1，b 大返回 -1，一样大返回 0
    static int compare(const HandResult& a, const HandResult& b);

private:
    // 判断 3 张普通牌（没有王）的牌型
    static HandResult evaluateNormal(const Card& c1, const Card& c2, const Card& c3);

    // 枚举：把王挨个变成每种普通牌，找出最大的牌型
    static void enumerateJokers(std::vector<Card>& cards, int idx, HandResult& best);
};

#endif // HAND_EVALUATOR_H
