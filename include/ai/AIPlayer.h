#ifndef AI_PLAYER_H
#define AI_PLAYER_H

#include "core/Card.h"
#include <string>
#include <vector>

// 职责: 拿到 9 张手牌, 决策如何分成 3 道, 输出 order[9] 交给 Player::arrangeByOrder
// 难度分级:
// Random 随机分组
// Greedy 胜率表贪心: 枚举全部 1680 种分组, 选"三组胜率期望"最高的
// MonteCarlo 蒙特卡洛模拟: 对候选分组模拟对手, 选期望赢池数最高
// 胜率表: 离线生成, 游戏启动时由 GameApp 加载
// 对所有 C=24804 种三张牌组合, 精确计算 vs 随机对手的胜率
class AIPlayer {
public:
    enum class Difficulty {
        Random = 0,  // 随机
        Greedy = 1,  // 贪心
        MonteCarlo = 2  // 蒙特卡洛
    };

  // 真人玩家的三种打法风格
    enum class Style {
        Balanced = 0,  // 均衡型: 三组胜率总和优先
        Aggressive = 1,  // 激进型: 偏爱把强牌集中成一组超强牌
        Conservative = 2  // 稳健型: 偏爱三组均衡, 不让任何一组太弱
    };

  // 核心决策接口: 给定 9 张手牌, 输出 order[9]
  // order[0..2]=头道手牌下标, order[3..5]=中道, order[6..8]=尾道
  // playerCount: 本局玩家总数
    static void decideOrder(const Card* hand, int playerCount,
                            Difficulty diff, int order[9]);

  // 人性化决策入口: 带风格 + 失误率
  // noise: 0.0~1.0, 0=最理性, 1=很随意
  // 返回后可用 diversityOf 检验打法多样性
    static void decideOrderStyled(const Card* hand, int playerCount,
                                  Difficulty diff, Style style, float noise,
                                  int order[9]);

    static void setProfile(Difficulty d, Style s, float noise);  // 一键设置
    static void setNoise(float n);  // 自动夹到 [0,1]
    static Difficulty difficulty();  // 当前难度

  // 三档难度枚举
    static const char* difficultyName(Difficulty d);  // "随机"/"普通"/"困难"
    static bool difficultyFromKey(const std::string& key, Difficulty& out);

  // 用"当前配置"决策: 界面层把 decideOrderStyled 换成这一个即可
    static void decideOrderAuto(const Card* hand, int playerCount, int order[9]);

  // 是否已有"生效配置": 界面调过 setProfile... 或设了环境变量
  // 为 true 时 decideOrder/decideOrderStyled 以配置为准
    static bool userProfileSet();

  // 人性化检验: 同一手牌决策 N 次, 返回出现过的不同分组方案数
  // 理性AI -> 接近 1; 随机AI -> 接近 1680; 人性化AI -> 中间值
    static int diversityOf(const Card* hand, int playerCount,
                           Difficulty diff, Style style, float noise,
                           int trials);

  // 胜率表: 离线生成, 游戏启动时由 GameApp 加载
    static bool loadWinRateTable(const std::string& path);
  // 胜率表: 离线生成, 保存到 path
    static bool generateWinRateTable(const std::string& path, bool verbose = true);

  // 单组 3 张牌 vs 随机对手的胜率
    static float winRateOf(const Card& a, const Card& b, const Card& c);

private:
    static std::vector<float> s_winrate;  // 24804 个组合的胜率
    static bool s_loaded;

  // 当前配置
    static Difficulty s_difficulty;
    static Style s_style;
    static float s_noise;
    static bool s_profileSet;  // 是否被界面/环境变量显式设置过
    static bool applyEnvConfigOnce();  // 首次调用时读一次环境变量

  // 牌 -> 全局编号 0~53
    static int cardId(const Card& c);
  // 组合编号: 3 张牌的全局编号 -> 0~24803
    static int combIndex(int a, int b, int c);

  // 枚举 9 张牌的全部 1680 种分组, 对每种调用 fn
  // line* 是长度为 3 的 int 数组
    template <typename Fn>
    static void forEachSplit(const Card* hand, Fn fn);

  // 贪心分组, 供 decideOrder 与蒙特卡洛对手模型复用
    static float bestGreedy(const Card* hand, int opponents, int order[9]);
  // 单组评分: 赢过所有对手的概率 ≈ winrate^opponents
    static float groupScore(const Card* hand, int idx0, int idx1, int idx2, int opponents);
  // 蒙特卡洛选组: 候选=贪心top-K, 模拟对手, 选期望赢池最高
    static void monteCarloChoose(const Card* hand, int opponents, int order[9], int sims, int topK);

  // 风格化贪心: 枚举全部分组, 按风格评分, 带失误率采样
    static int bestGreedyStyled(const Card* hand, int opponents, Style style, float noise, int order[9]);
  // 风格化蒙特卡洛: 候选按风格评分, 模拟后带失误率采样
    static void monteCarloStyled(const Card* hand, int opponents, int order[9], int sims, int topK, Style style, float noise);

  // 风格化评分: 对一组分法的三组胜率按风格聚合成总分
    static float styleScore(const float w[3], Style style);
  // 人性化选择: scores 降序后, 以 noise 概率从 top-K 里随机挑, 否则选最优
    static int humanPick(const std::vector<float>& scores, int topK, float noise);
};

#endif // AI_PLAYER_H
