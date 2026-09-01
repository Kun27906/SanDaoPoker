// ====== AI 对战评估器 v3 (成员B 训练工具) ======
// 功能: 胜率表抽查 / 难度对比 / 4人局 / 人性化多样性检验 / 风格对战
#include "ai/AIPlayer.h"
#include "core/Deck.h"
#include "core/HandEvaluator.h"
#include <cstdio>
#include <cstdlib>
#include <vector>

static void playOneRound(Deck& deck, int nPlayers,
                         const AIPlayer::Difficulty* diffs, int* scores) {
    deck.shuffle();
    std::vector<std::vector<Card>> hands = deck.deal(nPlayers);
    int orders[6][9];
    for (int p = 0; p < nPlayers; p++) {
        AIPlayer::decideOrder(hands[p].data(), nPlayers, diffs[p], orders[p]);
    }
    for (int line = 0; line < 3; line++) {
        HandResult hr[6];
        for (int p = 0; p < nPlayers; p++) {
            const int* o = orders[p];
            std::vector<Card> c = { hands[p][o[line*3]], hands[p][o[line*3+1]], hands[p][o[line*3+2]] };
            hr[p] = HandEvaluator::evaluate(c);
        }
        for (int p = 0; p < nPlayers; p++) {
            bool best = true;
            for (int q = 0; q < nPlayers; q++) {
                if (p == q) continue;
                if (HandEvaluator::compare(hr[p], hr[q]) < 0) { best = false; break; }
            }
            if (best) scores[p]++;
        }
    }
}

static const char* diffName(AIPlayer::Difficulty d) {
    switch (d) {
        case AIPlayer::Difficulty::Random: return "随机";
        case AIPlayer::Difficulty::Greedy: return "贪心";
        case AIPlayer::Difficulty::MonteCarlo: return "蒙特卡洛";
    }
    return "?";
}

static void sanityCheck() {
    std::printf("\n===== 胜率表正确性抽查 =====\n");
    auto wr = [](Suit s1, Rank r1, Suit s2, Rank r2, Suit s3, Rank r3) {
        return AIPlayer::winRateOf(Card(s1, r1), Card(s2, r2), Card(s3, r3));
    };
    std::printf("  豹子 AAA        : %.1f%%\n", 100.0 * wr(Suit::Spade, Rank::Ace, Suit::Heart, Rank::Ace, Suit::Club, Rank::Ace));
    std::printf("  同花顺 AKQ      : %.1f%%\n", 100.0 * wr(Suit::Spade, Rank::Ace, Suit::Spade, Rank::King, Suit::Spade, Rank::Queen));
    std::printf("  对子 QQ8        : %.1f%%\n", 100.0 * wr(Suit::Spade, Rank::Queen, Suit::Heart, Rank::Queen, Suit::Club, Rank::Eight));
    std::printf("  散牌 2 7 K      : %.1f%%\n", 100.0 * wr(Suit::Spade, Rank::Two, Suit::Heart, Rank::Seven, Suit::Club, Rank::King));
    std::printf("  异花 2 3 5      : %.1f%%\n", 100.0 * wr(Suit::Spade, Rank::Two, Suit::Heart, Rank::Three, Suit::Club, Rank::Five));
    std::printf("  单王(小王)+AA   : %.1f%%\n", 100.0 * AIPlayer::winRateOf(Card(Rank::SmallJoker), Card(Suit::Spade, Rank::Ace), Card(Suit::Heart, Rank::Ace)));
}

// ====== 人性化检验 1: 同一手牌决策 200 次的方案多样性 ======
static void diversityTest() {
    std::printf("\n===== 人性化检验: 同一手牌决策 200 次的方案多样性 =====\n");
    Card hand[9] = {
        Card(Suit::Spade, Rank::Ace), Card(Suit::Heart, Rank::King),
        Card(Suit::Club, Rank::Queen), Card(Suit::Diamond, Rank::Jack),
        Card(Suit::Spade, Rank::Ten), Card(Rank::SmallJoker),
        Card(Rank::BigJoker), Card(Suit::Heart, Rank::Nine),
        Card(Suit::Club, Rank::Eight)
    };
    struct Cfg { const char* name; AIPlayer::Difficulty d; AIPlayer::Style s; float noise; };
    Cfg cfgs[] = {
        { "理性贪心(noise=0)",    AIPlayer::Difficulty::Greedy, AIPlayer::Style::Balanced, 0.0f },
        { "人性化贪心(noise=.3)", AIPlayer::Difficulty::Greedy, AIPlayer::Style::Balanced, 0.3f },
        { "人性化贪心(noise=.6)", AIPlayer::Difficulty::Greedy, AIPlayer::Style::Balanced, 0.6f },
        { "激进贪心(noise=.3)",   AIPlayer::Difficulty::Greedy, AIPlayer::Style::Aggressive, 0.3f },
        { "稳健贪心(noise=.3)",   AIPlayer::Difficulty::Greedy, AIPlayer::Style::Conservative, 0.3f },
        { "纯随机",               AIPlayer::Difficulty::Random, AIPlayer::Style::Balanced, 1.0f },
    };
    for (auto& c : cfgs) {
        int n = AIPlayer::diversityOf(hand, 4, c.d, c.s, c.noise, 200);
        std::printf("  %-22s: %d 种方案\n", c.name, n);
    }
    std::printf("  (说明: 1680=完全随机, 1=每次完全一样; 人性化应在两者之间)\n");
}

// ====== 人性化检验 2: 风格对战 ======
static void styleBattle(int rounds) {
    std::printf("\n===== 风格对战 (%d 局, 2人局, 贪心难度) =====\n", rounds);
    AIPlayer::Style styles[3] = {
        AIPlayer::Style::Balanced, AIPlayer::Style::Aggressive, AIPlayer::Style::Conservative
    };
    const char* names[3] = { "均衡", "激进", "稳健" };
    for (int i = 0; i < 3; i++) {
        for (int j = i + 1; j < 3; j++) {
            int wins[2] = {0, 0}, draws = 0;
            long long lines[2] = {0, 0};
            for (int r = 0; r < rounds; r++) {
                int sc[2] = {0, 0};
                Deck deck;
                deck.shuffle();
                auto h = deck.deal(2);
                Card hands[2][9];
                for (int p = 0; p < 2; p++) for (int k = 0; k < 9; k++) hands[p][k] = h[p][k];
                int o[2][9];
                AIPlayer::decideOrderStyled(hands[0], 2, AIPlayer::Difficulty::Greedy, styles[i], 0.0f, o[0]);
                AIPlayer::decideOrderStyled(hands[1], 2, AIPlayer::Difficulty::Greedy, styles[j], 0.0f, o[1]);
                for (int line = 0; line < 3; line++) {
                    HandResult hr[2];
                    for (int p = 0; p < 2; p++) {
                        std::vector<Card> c = { hands[p][o[p][line*3]], hands[p][o[p][line*3+1]], hands[p][o[p][line*3+2]] };
                        hr[p] = HandEvaluator::evaluate(c);
                    }
                    for (int p = 0; p < 2; p++) {
                        if (HandEvaluator::compare(hr[p], hr[1-p]) > 0) sc[p]++;
                    }
                }
                lines[0] += sc[0]; lines[1] += sc[1];
                if (sc[0] > sc[1]) wins[0]++;
                else if (sc[1] > sc[0]) wins[1]++;
                else draws++;
            }
            std::printf("  %s vs %s : %d-%d-%d (%.1f%%:%.1f%%)  平均道数 %.3f:%.3f\n",
                names[i], names[j], wins[0], draws, wins[1],
                100.0 * wins[0] / rounds, 100.0 * wins[1] / rounds,
                (double)lines[0] / rounds, (double)lines[1] / rounds);
        }
    }
}

int main(int argc, char** argv) {
    int rounds = 3000;
    if (argc > 1) rounds = atoi(argv[1]);

    if (!AIPlayer::loadWinRateTable("assets/ai/winrate.bin")) {
        std::printf("警告: 未找到胜率表!\n");
    } else {
        std::printf("胜率表已加载\n");
    }
    sanityCheck();

    AIPlayer::Difficulty pairs[][2] = {
        { AIPlayer::Difficulty::Random,     AIPlayer::Difficulty::Greedy },
        { AIPlayer::Difficulty::Greedy,     AIPlayer::Difficulty::MonteCarlo },
        { AIPlayer::Difficulty::Random,     AIPlayer::Difficulty::MonteCarlo },
    };
    std::printf("\n===== 2人单挑 (%d 局) =====\n", rounds);
    for (auto& pair : pairs) {
        int wins[2] = {0, 0}, draws = 0;
        long long lines[2] = {0, 0};
        for (int r = 0; r < rounds; r++) {
            int sc[2] = {0, 0};
            Deck deck;
            AIPlayer::Difficulty d[2] = { pair[0], pair[1] };
            playOneRound(deck, 2, d, sc);
            lines[0] += sc[0]; lines[1] += sc[1];
            if (sc[0] > sc[1]) wins[0]++;
            else if (sc[1] > sc[0]) wins[1]++;
            else draws++;
        }
        std::printf("  %-6s vs %-6s : %d-%d-%d (%.1f%%:%.1f%%)  平均道数 %.3f:%.3f\n",
            diffName(pair[0]), diffName(pair[1]),
            wins[0], draws, wins[1],
            100.0 * wins[0] / rounds, 100.0 * wins[1] / rounds,
            (double)lines[0] / rounds, (double)lines[1] / rounds);
    }

    int n4 = rounds / 2;
    std::printf("\n===== 4人局平均赢道数 (%d 局) =====\n", n4);
    AIPlayer::Difficulty all4[] = {
        AIPlayer::Difficulty::Random, AIPlayer::Difficulty::Greedy,
        AIPlayer::Difficulty::MonteCarlo, AIPlayer::Difficulty::Greedy
    };
    long long total[4] = {0,0,0,0};
    for (int r = 0; r < n4; r++) {
        int sc[4] = {0,0,0,0};
        Deck deck;
        playOneRound(deck, 4, all4, sc);
        for (int p = 0; p < 4; p++) total[p] += sc[p];
    }
    for (int p = 0; p < 4; p++) {
        std::printf("  %-6s: 平均 %.3f 道/局\n", diffName(all4[p]), (double)total[p] / n4);
    }

    diversityTest();
    styleBattle(rounds / 2);

    std::printf("\n评估完成!\n");
    return 0;
}