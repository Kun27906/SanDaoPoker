#include "core/HandEvaluator.h"
#include <algorithm>

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

HandResult HandEvaluator::evaluate(const std::vector<Card>& three) {
    // 枚举王时需要改牌, 故复制一份
    std::vector<Card> cards = three;

    int jokerCount = 0;
    for (const Card& c : cards) {
        if (c.isJoker()) jokerCount++;
    }

    if (jokerCount == 0) {
        return evaluateNormal(cards[0], cards[1], cards[2]);
    }

    HandResult best;
    enumerateJokers(cards, 0, best);
    return best;
}

// 大王变红色任意点数, 小王变黑色任意点数; 最多两张王, 枚举量 26 x 26
void HandEvaluator::enumerateJokers(std::vector<Card>& cards, int idx, HandResult& best) {
    while (idx < 3 && !cards[idx].isJoker()) {
        idx++;
    }

    if (idx >= 3) {
        HandResult r = evaluateNormal(cards[0], cards[1], cards[2]);
        if (compare(r, best) > 0) {
            best = r;
        }
        return;
    }

    const bool isBig = cards[idx].isBigJoker();
    const Card original = cards[idx];

    for (int r = 2; r <= 14; r++) {
        for (int s = 0; s < 4; s++) {
            if (isBig && (s == 0 || s == 2)) continue;   // 大王不变黑
            if (!isBig && (s == 1 || s == 3)) continue;  // 小王不变红
            cards[idx] = Card(static_cast<Suit>(s), static_cast<Rank>(r));
            enumerateJokers(cards, idx + 1, best);
        }
    }

    // 恢复王, 否则后续组合会乱
    cards[idx] = original;
}

HandResult HandEvaluator::evaluateNormal(const Card& c1, const Card& c2, const Card& c3) {
    Card cards[3] = { c1, c2, c3 };
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2 - i; j++) {
            if (cards[j].getRankValue() > cards[j + 1].getRankValue()) {
                std::swap(cards[j], cards[j + 1]);
            }
        }
    }

    const int v1 = cards[0].getRankValue();
    const int v2 = cards[1].getRankValue();
    const int v3 = cards[2].getRankValue();

    const bool sameSuit = (cards[0].getSuitValue() == cards[1].getSuitValue())
                       && (cards[1].getSuitValue() == cards[2].getSuitValue());

    HandResult r;

    if (v1 == v2 && v2 == v3) {
        r.type = HandType::ThreeOfAKind;
        r.keys[0] = v1;
        return r;
    }

    // A23 算顺子, 且最小
    bool isStraight = false;
    int straightKey = v3;
    if (v2 == v1 + 1 && v3 == v2 + 1) {
        isStraight = true;
    } else if (v1 == 2 && v2 == 3 && v3 == 14) {
        isStraight = true;
        straightKey = 3;
    }

    if (sameSuit) {
        if (isStraight) {
            r.type = HandType::StraightFlush;
            r.keys[0] = straightKey;
            return r;
        }
        r.type = HandType::Flush;
        r.keys[0] = v3; r.keys[1] = v2; r.keys[2] = v1;
        return r;
    }

    if (isStraight) {
        r.type = HandType::Straight;
        r.keys[0] = straightKey;
        return r;
    }

    if (v1 == v2) {
        r.type = HandType::Pair;
        r.keys[0] = v1;
        r.keys[1] = v3;
        return r;
    }
    if (v2 == v3) {
        r.type = HandType::Pair;
        r.keys[0] = v2;
        r.keys[1] = v1;
        return r;
    }

    r.type = HandType::HighCard;
    r.keys[0] = v3; r.keys[1] = v2; r.keys[2] = v1;

    // 异花 235 可吃豹子
    if (v1 == 2 && v2 == 3 && v3 == 5 && !sameSuit) {
        r.is235 = true;
    }
    return r;
}

int HandEvaluator::compare(const HandResult& a, const HandResult& b) {
    // 异花 235 吃豹子
    if (a.is235 && b.type == HandType::ThreeOfAKind) return 1;
    if (b.is235 && a.type == HandType::ThreeOfAKind) return -1;

    if (a.type != b.type) {
        return (static_cast<int>(a.type) > static_cast<int>(b.type)) ? 1 : -1;
    }

    for (int i = 0; i < 3; i++) {
        if (a.keys[i] != b.keys[i]) {
            return (a.keys[i] > b.keys[i]) ? 1 : -1;
        }
    }

    return 0;
}
