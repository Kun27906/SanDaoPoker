#include "core/HandEvaluator.h"
#include <algorithm>

// ====== 牌型的中文名字 ======

std::string HandResult::name() const {
    switch (type) {
        case HandType::ThreeOfAKind:  return "豹子";
        case HandType::StraightFlush: return "同花顺";
        case HandType::Flush:         return "金花";
        case HandType::Straight:      return "顺子";
        case HandType::Pair:          return "对子";
        default:                      return is235 ? "异花235" : "散牌";
    }
}

// ====== 判断 3 张牌是什么牌型 ======

HandResult HandEvaluator::evaluate(const std::vector<Card>& three) {
    // 复制一份，因为枚举王的时候要改牌
    std::vector<Card> cards = three;

    // 先数一数有几个王
    int jokerCount = 0;
    for (const Card& c : cards) {
        if (c.isJoker()) jokerCount++;
    }

    // 没有王：直接判断
    if (jokerCount == 0) {
        return evaluateNormal(cards[0], cards[1], cards[2]);
    }

    // 有王：枚举王变成各种牌，找最大的牌型
    HandResult best;  // 初始是最小的散牌
    enumerateJokers(cards, 0, best);
    return best;
}

// ====== 枚举：把王变成每种普通牌 ======
// 规则（按分工方案）：
//   大王（BigJoker）→ 只能变成 红色 花色（♥ 红桃 / ♦ 方块）的任意点数
//   小王（SmallJoker）→ 只能变成 黑色 花色（♠ 黑桃 / ♣ 梅花）的任意点数
// 枚举所有能变的牌，找出最大的牌型
// 最多 2 个王，枚举量 26 × 26 种，电脑算得飞快

void HandEvaluator::enumerateJokers(std::vector<Card>& cards, int idx, HandResult& best) {
    // 从 idx 开始找第一张王
    while (idx < 3 && !cards[idx].isJoker()) {
        idx++;
    }

    // 没有王了：判断当前这手牌，和最好的比一比
    if (idx >= 3) {
        HandResult r = evaluateNormal(cards[0], cards[1], cards[2]);
        if (compare(r, best) > 0) {
            best = r;  // 比当前最好的还大，就记下来
        }
        return;
    }

    // 这张王能变成哪些花色？
    bool isBig = cards[idx].isBigJoker();   // 是大王吗
    Card original = cards[idx];             // 记住原来的王（递归完要恢复！）

    // 点数 2~A 都能变
    for (int r = 2; r <= 14; r++) {
        // 花色：大王只能变红（1红桃 3方块），小王只能变黑（0黑桃 2梅花）
        for (int s = 0; s < 4; s++) {
            if (isBig && (s == 0 || s == 2)) continue;  // 大王不能变黑色
            if (!isBig && (s == 1 || s == 3)) continue; // 小王不能变红色
            cards[idx] = Card(static_cast<Suit>(s), static_cast<Rank>(r));
            enumerateJokers(cards, idx + 1, best);  // 继续处理下一张王
        }
    }

    // 重要！把王变回来，否则后面的组合会乱套（双王时尤其明显）
    cards[idx] = original;
}

// ====== 判断 3 张普通牌（没有王）的牌型 ======

HandResult HandEvaluator::evaluateNormal(const Card& c1, const Card& c2, const Card& c3) {
    // 先按点数从小到大排序
    Card cards[3] = { c1, c2, c3 };
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2 - i; j++) {
            if (cards[j].getRankValue() > cards[j + 1].getRankValue()) {
                std::swap(cards[j], cards[j + 1]);
            }
        }
    }

    // 取出三个点数（小、中、大）
    int v1 = cards[0].getRankValue();
    int v2 = cards[1].getRankValue();
    int v3 = cards[2].getRankValue();

    // 是不是同花色
    bool sameSuit = (cards[0].getSuitValue() == cards[1].getSuitValue())
                 && (cards[1].getSuitValue() == cards[2].getSuitValue());

    HandResult r;  // 结果

    // ① 豹子：三张点数一样
    if (v1 == v2 && v2 == v3) {
        r.type = HandType::ThreeOfAKind;
        r.keys[0] = v1;  // 比大小就看点数
        return r;
    }

    // ② 判断是不是顺子（点数连续）
    //    特殊：A-2-3 也算顺子（A 当 1 用），它是顺子里最小的
    bool isStraight = false;
    int straightKey = v3;  // 顺子比大小看最大点数（A23 顺子最大是 3）
    if (v2 == v1 + 1 && v3 == v2 + 1) {
        isStraight = true;
    } else if (v1 == 2 && v2 == 3 && v3 == 14) {
        isStraight = true;
        straightKey = 3;  // A23：A 当 1，所以最大是 3
    }

    // ③ 同花色：可能是同花顺，也可能是金花
    if (sameSuit) {
        if (isStraight) {
            r.type = HandType::StraightFlush;  // 又同花又连续 = 同花顺
            r.keys[0] = straightKey;
            return r;
        }
        r.type = HandType::Flush;              // 只同花不连续 = 金花
        r.keys[0] = v3; r.keys[1] = v2; r.keys[2] = v1;
        return r;
    }

    // ④ 顺子（不同花色）
    if (isStraight) {
        r.type = HandType::Straight;
        r.keys[0] = straightKey;
        return r;
    }

    // ⑤ 对子：两张一样
    if (v1 == v2) {
        r.type = HandType::Pair;
        r.keys[0] = v1;  // 先比对子的点数
        r.keys[1] = v3;  // 再比剩下那张
        return r;
    }
    if (v2 == v3) {
        r.type = HandType::Pair;
        r.keys[0] = v2;
        r.keys[1] = v1;
        return r;
    }

    // ⑥ 散牌：啥都不是
    r.type = HandType::HighCard;
    r.keys[0] = v3; r.keys[1] = v2; r.keys[2] = v1;

    // 特殊神牌：异花 2-3-5（不同花色的 2、3、5）
    // 它本身是最小的散牌，但能吃掉豹子！
    if (v1 == 2 && v2 == 3 && v3 == 5 && !sameSuit) {
        r.is235 = true;
    }
    return r;
}

// ====== 比较两手牌的大小 ======

int HandEvaluator::compare(const HandResult& a, const HandResult& b) {
    // 特殊规则：异花 235 吃豹子！
    if (a.is235 && b.type == HandType::ThreeOfAKind) return 1;
    if (b.is235 && a.type == HandType::ThreeOfAKind) return -1;

    // 先比牌型等级（豹子 > 同花顺 > 金花 > 顺子 > 对子 > 散牌）
    if (a.type != b.type) {
        return (static_cast<int>(a.type) > static_cast<int>(b.type)) ? 1 : -1;
    }

    // 牌型一样，比点数（从大到小逐位比）
    for (int i = 0; i < 3; i++) {
        if (a.keys[i] != b.keys[i]) {
            return (a.keys[i] > b.keys[i]) ? 1 : -1;
        }
    }

    return 0;  // 完全一样大，平局
}
