// ====== 人性化快速检验 (成员B) ======
// 只测: 决策多样性 + 风格对战 (都是贪心, 秒出结果)
#include "ai/AIPlayer.h"
#include "core/Deck.h"
#include "core/HandEvaluator.h"
#include <cstdio>
#include <vector>

int main() {
    if (!AIPlayer::loadWinRateTable("assets/ai/winrate.bin")) {
        std::printf("未找到胜率表!\n"); return 1;
    }

    std::printf("===== 人性化检验: 同一手牌决策 200 次的方案多样性 =====\n");
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
    std::printf("  (1680=完全随机, 1=每次一样; 人性化应居中)\n");

    std::printf("\n===== 风格对战 (2人局, 贪心, 500局) =====\n");
    AIPlayer::Style styles[3] = { AIPlayer::Style::Balanced, AIPlayer::Style::Aggressive, AIPlayer::Style::Conservative };
    const char* names[3] = { "均衡", "激进", "稳健" };
    for (int i = 0; i < 3; i++) for (int j = i + 1; j < 3; j++) {
        int wins[2] = {0,0}, draws = 0; long long lines[2] = {0,0};
        for (int r = 0; r < 500; r++) {
            int sc[2] = {0,0};
            Deck deck; deck.shuffle();
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
                for (int p = 0; p < 2; p++) if (HandEvaluator::compare(hr[p], hr[1-p]) > 0) sc[p]++;
            }
            lines[0] += sc[0]; lines[1] += sc[1];
            if (sc[0] > sc[1]) wins[0]++; else if (sc[1] > sc[0]) wins[1]++; else draws++;
        }
        std::printf("  %s vs %s : %d-%d-%d (%.1f%%:%.1f%%)  平均道数 %.3f:%.3f\n",
            names[i], names[j], wins[0], draws, wins[1],
            100.0*wins[0]/500, 100.0*wins[1]/500,
            (double)lines[0]/500, (double)lines[1]/500);
    }
    std::printf("\n完成!\n");
    return 0;
}