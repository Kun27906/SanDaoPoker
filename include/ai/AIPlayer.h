#ifndef AI_PLAYER_H
#define AI_PLAYER_H

#include "core/Card.h"
#include <string>
#include <vector>

// 分道决策
class AIPlayer {
public:
    enum class Difficulty {
        Random = 0,
        Greedy = 1,
        MonteCarlo = 2
    };

    enum class Style {
        Balanced = 0,
        Aggressive = 1,
        Conservative = 2
    };

    // order[0..2] 头道, [3..5] 中道, [6..8] 尾道
    static void decideOrder(const Card* hand, int playerCount,
                            Difficulty diff, int order[9]);

    static void decideOrderStyled(const Card* hand, int playerCount,
                                  Difficulty diff, Style style, float noise,
                                  int order[9]);

    static void setProfile(Difficulty d, Style s, float noise);
    static void setNoise(float n);
    static Difficulty difficulty();

    static const char* difficultyName(Difficulty d);
    static bool difficultyFromKey(const std::string& key, Difficulty& out);

    static void decideOrderAuto(const Card* hand, int playerCount, int order[9]);

    static bool userProfileSet();

    // 同一手牌决策 trials 次, 返回不同分组方案数
    static int diversityOf(const Card* hand, int playerCount,
                           Difficulty diff, Style style, float noise,
                           int trials);

    static bool loadWinRateTable(const std::string& path);
    static bool generateWinRateTable(const std::string& path, bool verbose = true);

    static float winRateOf(const Card& a, const Card& b, const Card& c);

private:
    static std::vector<float> s_winrate;
    static bool s_loaded;

    static Difficulty s_difficulty;
    static Style s_style;
    static float s_noise;
    static bool s_profileSet;
    static bool applyEnvConfigOnce();

    static int cardId(const Card& c);
    static int combIndex(int a, int b, int c);

    template <typename Fn>
    static void forEachSplit(const Card* hand, Fn fn);

    static float bestGreedy(const Card* hand, int opponents, int order[9]);
    static float groupScore(const Card* hand, int idx0, int idx1, int idx2, int opponents);
    static void monteCarloChoose(const Card* hand, int opponents, int order[9], int sims, int topK);

    static int bestGreedyStyled(const Card* hand, int opponents, Style style, float noise, int order[9]);
    static void monteCarloStyled(const Card* hand, int opponents, int order[9], int sims, int topK, Style style, float noise);

    static float styleScore(const float w[3], Style style);
    static int humanPick(const std::vector<float>& scores, int topK, float noise);
};

#endif
