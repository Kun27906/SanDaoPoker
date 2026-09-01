#ifndef AI_PLAYER_H
#define AI_PLAYER_H

#include "core/Card.h"
#include <string>
#include <vector>

// ====== AI 玩家 (成员B: src/ai) ======
// 职责: 拿到 9 张手牌, 决策如何分成 3 道, 输出 order[9] 交给 Player::arrangeByOrder
//
// 难度分级:
//   Random      随机分组 (陪练/基线)
//   Greedy      胜率表贪心: 枚举全部 1680 种分组, 选"三组胜率期望"最高的
//   MonteCarlo  蒙特卡洛模拟: 对候选分组模拟对手(对手也用贪心), 选期望赢池数最高
//
// 胜率表: 离线生成 (tools/gen_winrate_table), 运行时加载
//   对所有 C(54,3)=24804 种三张牌组合, 精确计算 vs 随机对手(剩余51张任取3)的胜率
class AIPlayer {
public:
    enum class Difficulty {
        Random = 0,     // 随机
        Greedy = 1,     // 贪心 (普通)
        MonteCarlo = 2  // 蒙特卡洛 (困难)
    };

    // ====== 人性化风格 ======
    // 真人玩家的三种打法风格
    enum class Style {
        Balanced = 0,      // 均衡型: 三组胜率总和优先 (理性打法)
        Aggressive = 1,    // 激进型: 偏爱把强牌集中成一组超强牌
        Conservative = 2   // 稳健型: 偏爱三组均衡, 不让任何一组太弱
    };

    // 核心决策接口: 给定 9 张手牌, 输出 order[9]
    // order[0..2]=头道手牌下标, order[3..5]=中道, order[6..8]=尾道
    // playerCount: 本局玩家总数 (用于估计"赢过所有对手"的概率)
    static void decideOrder(const Card* hand, int playerCount,
                            Difficulty diff, int order[9]);

    // 人性化决策入口: 带风格 + 失误率
    // noise: 0.0~1.0, 0=最理性(每次都选最优), 1=很随意(经常在次优方案里随机挑)
    // 返回后可用 diversityOf(hand, ...) 检验打法多样性
    static void decideOrderStyled(const Card* hand, int playerCount,
                                  Difficulty diff, Style style, float noise,
                                  int order[9]);

    // 人性化检验: 同一手牌决策 N 次, 返回出现过的不同分组方案数
    // 理性AI -> 接近 1; 随机AI -> 接近 1680; 人性化AI -> 中间值
    static int diversityOf(const Card* hand, int playerCount,
                           Difficulty diff, Style style, float noise,
                           int trials);

    // 胜率表: 运行时加载 (winrate.bin)
    static bool loadWinRateTable(const std::string& path);
    // 胜率表: 离线生成 (训练), 保存到 path
    static bool generateWinRateTable(const std::string& path, bool verbose = true);
    // 是否已加载
    static bool hasWinRateTable() { return s_loaded; }

    // 单组 3 张牌 vs 随机对手的胜率 (查表; 未加载返回 -1)
    static float winRateOf(const Card& a, const Card& b, const Card& c);

private:
    static std::vector<float> s_winrate;  // 24804 个组合的胜率
    static bool s_loaded;

    // 牌 -> 全局编号 0~53 (与 Deck 生成顺序一致: 花色x点数, 王=52/53)
    static int cardId(const Card& c);
    // 组合编号: 3 张牌的全局编号 (调用方保证 a<b<c) -> 0~24803
    static int combIndex(int a, int b, int c);
    // 组合数 C(n,k), n<k 返回 0
    static long long combN(int n, int k);

    // 枚举 9 张牌的全部 1680 种分组 (头/中/尾有序), 对每种调用 fn(line0, line1, line2)
    // line* 是长度为 3 的 int 数组 (手牌下标)
    template <typename Fn>
    static void forEachSplit(const Card* hand, Fn fn);

    // 贪心分组 (返回最佳 order), 供 decideOrder 与蒙特卡洛对手模型复用
    static float bestGreedy(const Card* hand, int opponents, int order[9]);
    // 单组评分: 赢过所有对手的概率 ≈ winrate^opponents (查表失败时退回牌型打分)
    static float groupScore(const Card* hand, int idx0, int idx1, int idx2, int opponents);
    // 蒙特卡洛选组: 候选=贪心top-K, 模拟对手(同款贪心), 选期望赢池最高
    static void monteCarloChoose(const Card* hand, int opponents, int order[9], int sims, int topK);

    // 风格化贪心: 枚举全部分组, 按风格评分, 带失误率采样
    static int bestGreedyStyled(const Card* hand, int opponents, Style style, float noise, int order[9]);
    // 风格化蒙特卡洛: 候选按风格评分, 模拟后带失误率采样
    static void monteCarloStyled(const Card* hand, int opponents, int order[9], int sims, int topK, Style style, float noise);

    // 风格化评分: 对一组分法的三组胜率按风格聚合成总分
    static float styleScore(const float w[3], Style style);
    // 人性化选择: scores 降序后, 以 noise 概率从 top-K 里随机挑, 否则选最优
    static int humanPick(const std::vector<float>& scores, int topK, float noise);

    // 牌型打分 (无表时的退化方案): 豹子>顺金>金花>顺子>对子>散牌, 同型比点数
    static float fallbackScore(const Card& a, const Card& b, const Card& c);
};

#endif // AI_PLAYER_H