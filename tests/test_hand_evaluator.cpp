// ====== HandEvaluator 单元测试 ======
// 成员A的单元测试：覆盖全部特殊规则边界
// 运行方式：build 后执行 test_hand_evaluator.exe（或 ctest）
// 返回 0 = 全部通过；返回 1 = 有失败

#include "core/HandEvaluator.h"
#include "core/Card.h"
#include <iostream>
#include <vector>
#include <string>

// ====== 统计：通过了几个、失败了几个 ======
static int gPassed = 0;
static int gFailed = 0;

// ====== 检查一个测试用例 ======
// name：用例名字；ok：这个用例是否通过
static void check(const char* name, bool ok) {
    if (ok) {
        gPassed++;
        std::cout << "PASS  " << name << std::endl;
    } else {
        gFailed++;
        std::cout << "FAIL  " << name << std::endl;
    }
}

// ====== 造 3 张普通牌的小工具 ======
static std::vector<Card> make3(Suit s1, Rank r1, Suit s2, Rank r2, Suit s3, Rank r3) {
    std::vector<Card> v;
    v.push_back(Card(s1, r1));
    v.push_back(Card(s2, r2));
    v.push_back(Card(s3, r3));
    return v;
}

// ====== 造 2 王 + 1 普通牌的小工具 ======
static std::vector<Card> makeJokers(int jokers, Suit s, Rank r) {
    std::vector<Card> v;
    if (jokers >= 1) v.push_back(Card(Rank::SmallJoker));
    if (jokers >= 2) v.push_back(Card(Rank::BigJoker));
    v.push_back(Card(s, r));
    return v;
}

// ====== 主测试入口 ======
int main() {
    std::cout << "===== HandEvaluator 单元测试 =====" << std::endl;

    // ---------- 1. 基本牌型判定 ----------
    {
        std::vector<Card> h = make3(Suit::Spade, Rank::King, Suit::Heart, Rank::King, Suit::Club, Rank::King);
        check("KKK 是豹子", HandEvaluator::evaluate(h).type == HandType::ThreeOfAKind);
    }
    {
        std::vector<Card> h = make3(Suit::Spade, Rank::Seven, Suit::Spade, Rank::Eight, Suit::Spade, Rank::Nine);
        check("黑桃789 是同花顺", HandEvaluator::evaluate(h).type == HandType::StraightFlush);
    }
    {
        std::vector<Card> h = make3(Suit::Spade, Rank::Two, Suit::Spade, Rank::Five, Suit::Spade, Rank::Nine);
        check("黑桃259 是金花", HandEvaluator::evaluate(h).type == HandType::Flush);
    }
    {
        std::vector<Card> h = make3(Suit::Heart, Rank::Four, Suit::Spade, Rank::Five, Suit::Diamond, Rank::Six);
        check("456 是顺子", HandEvaluator::evaluate(h).type == HandType::Straight);
    }
    {
        std::vector<Card> h = make3(Suit::Spade, Rank::Five, Suit::Heart, Rank::Five, Suit::Club, Rank::Nine);
        check("559 是对子", HandEvaluator::evaluate(h).type == HandType::Pair);
    }
    {
        std::vector<Card> h = make3(Suit::Spade, Rank::Two, Suit::Heart, Rank::Seven, Suit::Club, Rank::King);
        check("27K 是散牌", HandEvaluator::evaluate(h).type == HandType::HighCard);
    }

    // ---------- 2. 牌型大小顺序 ----------
    {
        // 豹子 > 同花顺
        std::vector<Card> trio = make3(Suit::Spade, Rank::King, Suit::Heart, Rank::King, Suit::Club, Rank::King);
        std::vector<Card> sfl = make3(Suit::Spade, Rank::Seven, Suit::Spade, Rank::Eight, Suit::Spade, Rank::Nine);
        check("豹子 > 同花顺", HandEvaluator::compare(HandEvaluator::evaluate(trio), HandEvaluator::evaluate(sfl)) > 0);
    }
    {
        // 同花顺 > 金花
        std::vector<Card> sfl = make3(Suit::Spade, Rank::Seven, Suit::Spade, Rank::Eight, Suit::Spade, Rank::Nine);
        std::vector<Card> fl = make3(Suit::Spade, Rank::Two, Suit::Spade, Rank::Five, Suit::Spade, Rank::Nine);
        check("同花顺 > 金花", HandEvaluator::compare(HandEvaluator::evaluate(sfl), HandEvaluator::evaluate(fl)) > 0);
    }
    {
        // 金花 > 顺子
        std::vector<Card> fl = make3(Suit::Spade, Rank::Two, Suit::Spade, Rank::Five, Suit::Spade, Rank::Nine);
        std::vector<Card> st = make3(Suit::Heart, Rank::Four, Suit::Spade, Rank::Five, Suit::Diamond, Rank::Six);
        check("金花 > 顺子", HandEvaluator::compare(HandEvaluator::evaluate(fl), HandEvaluator::evaluate(st)) > 0);
    }
    {
        // 顺子 > 对子
        std::vector<Card> st = make3(Suit::Heart, Rank::Four, Suit::Spade, Rank::Five, Suit::Diamond, Rank::Six);
        std::vector<Card> pr = make3(Suit::Spade, Rank::Five, Suit::Heart, Rank::Five, Suit::Club, Rank::Nine);
        check("顺子 > 对子", HandEvaluator::compare(HandEvaluator::evaluate(st), HandEvaluator::evaluate(pr)) > 0);
    }
    {
        // 对子 > 散牌
        std::vector<Card> pr = make3(Suit::Spade, Rank::Five, Suit::Heart, Rank::Five, Suit::Club, Rank::Nine);
        std::vector<Card> hc = make3(Suit::Spade, Rank::Two, Suit::Heart, Rank::Seven, Suit::Club, Rank::King);
        check("对子 > 散牌", HandEvaluator::compare(HandEvaluator::evaluate(pr), HandEvaluator::evaluate(hc)) > 0);
    }

    // ---------- 3. 特殊规则1：异花235 吃豹子 ----------
    {
        std::vector<Card> c235 = make3(Suit::Spade, Rank::Two, Suit::Heart, Rank::Three, Suit::Club, Rank::Five);
        std::vector<Card> trio = make3(Suit::Spade, Rank::King, Suit::Heart, Rank::King, Suit::Club, Rank::King);
        HandResult r235 = HandEvaluator::evaluate(c235);
        check("异花235 被识别", r235.is235);
        check("异花235 吃豹子", HandEvaluator::compare(r235, HandEvaluator::evaluate(trio)) > 0);
    }
    {
        // 同花 235 = 金花，不是特殊牌 → 打不过豹子
        std::vector<Card> flush235 = make3(Suit::Spade, Rank::Two, Suit::Spade, Rank::Three, Suit::Spade, Rank::Five);
        std::vector<Card> trio = make3(Suit::Spade, Rank::King, Suit::Heart, Rank::King, Suit::Club, Rank::King);
        HandResult r = HandEvaluator::evaluate(flush235);
        check("同花235 是金花不是异花235", r.type == HandType::Flush && !r.is235);
        check("同花235 打不过豹子", HandEvaluator::compare(r, HandEvaluator::evaluate(trio)) < 0);
    }
    {
        // 异花235 遇到散牌：按散牌比（235 很小）
        std::vector<Card> c235 = make3(Suit::Spade, Rank::Two, Suit::Heart, Rank::Three, Suit::Club, Rank::Five);
        std::vector<Card> hc = make3(Suit::Spade, Rank::Four, Suit::Heart, Rank::Seven, Suit::Club, Rank::King);
        check("异花235 打不过大散牌", HandEvaluator::compare(HandEvaluator::evaluate(c235), HandEvaluator::evaluate(hc)) < 0);
    }

    // ---------- 4. 特殊规则2：王当万能牌（大王=红、小王=黑） ----------
    {
        // 小王 + 红桃5 + 红桃6：小王只能变黑花色 → 最多顺子（♠7 或 ♣7）
        std::vector<Card> h = makeJokers(1, Suit::Heart, Rank::Five);
        h.push_back(Card(Suit::Heart, Rank::Six));
        HandResult r = HandEvaluator::evaluate(h);
        check("小王+红桃5+红桃6 = 顺子（不能变红凑同花顺）", r.type == HandType::Straight);
    }
    {
        // 大王 + 黑桃5 + 黑桃6：大王只能变红花色 → 最多顺子
        std::vector<Card> h;
        h.push_back(Card(Rank::BigJoker));
        h.push_back(Card(Suit::Spade, Rank::Five));
        h.push_back(Card(Suit::Spade, Rank::Six));
        HandResult r = HandEvaluator::evaluate(h);
        check("大王+黑桃5+黑桃6 = 顺子（不能变黑凑同花顺）", r.type == HandType::Straight);
    }
    {
        // 小王 + 黑桃5 + 黑桃5：小王变黑桃5 → 豹子555
        std::vector<Card> h;
        h.push_back(Card(Rank::SmallJoker));
        h.push_back(Card(Suit::Spade, Rank::Five));
        h.push_back(Card(Suit::Spade, Rank::Five));
        HandResult r = HandEvaluator::evaluate(h);
        check("小王+黑桃5+黑桃5 = 豹子", r.type == HandType::ThreeOfAKind);
    }
    {
        // 双王 + 任意牌：两个王变同点数 → 豹子
        std::vector<Card> h = makeJokers(2, Suit::Spade, Rank::Nine);
        HandResult r = HandEvaluator::evaluate(h);
        std::cout << "[diag] 双王+9 -> " << r.name() << " type=" << (int)r.type
                  << " keys=" << r.keys[0] << "," << r.keys[1] << "," << r.keys[2] << std::endl;
        check("双王+9 = 豹子", r.type == HandType::ThreeOfAKind);
    }
    {
        // 小王 + 红桃A + 红桃2：小王变黑桃3 → 顺子 A23（黑桃3，红桃A，红桃2）
        std::vector<Card> h;
        h.push_back(Card(Rank::SmallJoker));
        h.push_back(Card(Suit::Heart, Rank::Ace));
        h.push_back(Card(Suit::Heart, Rank::Two));
        HandResult r = HandEvaluator::evaluate(h);
        check("小王+红桃A+红桃2 = 顺子A23", r.type == HandType::Straight);
    }

    // ---------- 5. A23 特殊顺子 ----------
    {
        std::vector<Card> a23 = make3(Suit::Spade, Rank::Ace, Suit::Heart, Rank::Two, Suit::Club, Rank::Three);
        HandResult r = HandEvaluator::evaluate(a23);
        check("A23 是最小顺子", r.type == HandType::Straight);
    }
    {
        // A23 顺子 < 234 顺子
        std::vector<Card> a23 = make3(Suit::Spade, Rank::Ace, Suit::Heart, Rank::Two, Suit::Club, Rank::Three);
        std::vector<Card> c234 = make3(Suit::Spade, Rank::Two, Suit::Heart, Rank::Three, Suit::Club, Rank::Four);
        check("A23 < 234", HandEvaluator::compare(HandEvaluator::evaluate(a23), HandEvaluator::evaluate(c234)) < 0);
    }

    // ---------- 6. 同牌型比较（点数） ----------
    {
        std::vector<Card> kkk = make3(Suit::Spade, Rank::King, Suit::Heart, Rank::King, Suit::Club, Rank::King);
        std::vector<Card> aaa = make3(Suit::Spade, Rank::Ace, Suit::Heart, Rank::Ace, Suit::Club, Rank::Ace);
        check("AAA > KKK", HandEvaluator::compare(HandEvaluator::evaluate(aaa), HandEvaluator::evaluate(kkk)) > 0);
    }
    {
        std::vector<Card> pr55 = make3(Suit::Spade, Rank::Five, Suit::Heart, Rank::Five, Suit::Club, Rank::Nine);
        std::vector<Card> pr66 = make3(Suit::Spade, Rank::Six, Suit::Heart, Rank::Six, Suit::Club, Rank::Two);
        check("66对子 > 55对子", HandEvaluator::compare(HandEvaluator::evaluate(pr66), HandEvaluator::evaluate(pr55)) > 0);
    }

    // ---------- 7. 平局 ----------
    {
        std::vector<Card> a1 = make3(Suit::Spade, Rank::King, Suit::Heart, Rank::King, Suit::Club, Rank::King);
        std::vector<Card> a2 = make3(Suit::Diamond, Rank::King, Suit::Spade, Rank::King, Suit::Heart, Rank::King);
        check("豹子K vs 豹子K 平局", HandEvaluator::compare(HandEvaluator::evaluate(a1), HandEvaluator::evaluate(a2)) == 0);
    }

    // ---------- 结果汇总 ----------
    std::cout << "===== 结果：" << gPassed << " 通过，" << gFailed << " 失败 =====" << std::endl;
    if (gFailed > 0) {
        std::cout << "有测试失败！请检查。" << std::endl;
        return 1;
    }
    std::cout << "全部通过！" << std::endl;
    return 0;
}
