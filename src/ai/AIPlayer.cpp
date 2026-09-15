#include "ai/AIPlayer.h"
#include "core/HandEvaluator.h"
#include "core/Deck.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <thread>
#include <unordered_map>
#include <vector>

std::vector<float> AIPlayer::s_winrate;
bool AIPlayer::s_loaded = false;

AIPlayer::Difficulty AIPlayer::s_difficulty = AIPlayer::Difficulty::Greedy;
AIPlayer::Style AIPlayer::s_style = AIPlayer::Style::Balanced;
float AIPlayer::s_noise = 0.3f;
bool AIPlayer::s_profileSet = false;

static const int NUM_COMBO = 24804;     // C 全部三张牌组合
static const int NUM_OPPONENT = 20825;  // C 对手组合数
static const int SPLIT_COUNT = 1680;    // 9张牌分成3道的分组数

// 预计算组合表: 每个组合的, 牌型结果, 位掩码
struct ComboEntry {
    int id[3];                 // 牌全局编号, 升序
    unsigned long long mask;   // 位掩码
    HandResult hr;             // 牌型
};

static std::vector<ComboEntry> g_combos;   // 长度 24804
static std::unordered_map<unsigned long long, int> g_comboIndex; // -> idx

// 与 Deck 生成顺序一致: suit x rank, 然后 SmallJoker=52, BigJoker=53
int AIPlayer::cardId(const Card& c) {
    if (c.isSmallJoker()) return 52;
    if (c.isBigJoker()) return 53;
    return c.getSuitValue() * 13 + (c.getRankValue() - 2);
}

// 组合排名公式: C+C+C
int AIPlayer::combIndex(int a, int b, int c) {
    auto it = g_comboIndex.find(((unsigned long long)a << 12) | ((unsigned long long)b << 6) | (unsigned long long)c);
    return (it != g_comboIndex.end()) ? it->second : 0;
}

static Card cardFromId(int id) {
    if (id == 52) return Card(Rank::SmallJoker);
    if (id == 53) return Card(Rank::BigJoker);
    int s = id / 13;
    int r = id % 13 + 2;
    return Card(static_cast<Suit>(s), static_cast<Rank>(r));
}

static void buildComboTable() {
    if (!g_combos.empty()) return;
    g_combos.reserve(NUM_COMBO);
    int idx = 0;
    for (int a = 0; a < 54; a++) {
        for (int b = a + 1; b < 54; b++) {
            for (int c = b + 1; c < 54; c++) {
                ComboEntry e;
                e.id[0] = a; e.id[1] = b; e.id[2] = c;
                e.mask = (1ULL << a) | (1ULL << b) | (1ULL << c);
                std::vector<Card> three = { cardFromId(a), cardFromId(b), cardFromId(c) };
                e.hr = HandEvaluator::evaluate(three);
                g_combos.push_back(e);
                g_comboIndex[((unsigned long long)a << 12) | ((unsigned long long)b << 6) | (unsigned long long)c] = idx;
                idx++;
            }
        }
    }
}

// 对每个组合 i, 遍历所有与之不重叠的组合 j, 精确统计胜/平/负
// 平局算 0.5 胜率. 输出 24804 个 float 到二进制文件
bool AIPlayer::generateWinRateTable(const std::string& path, bool verbose) {
    buildComboTable();
    std::vector<float> table(NUM_COMBO, 0.0f);

    if (verbose) {
        std::printf("生成胜率表: %d 个组合, 每个 vs %d 个对手组合...\n", NUM_COMBO, NUM_OPPONENT);
        std::fflush(stdout);
    }

    // 并行: 分块到多个线程
    unsigned hw = std::thread::hardware_concurrency();
    if (hw < 2) hw = 2;
    if (hw > 8) hw = 8;
    std::vector<std::thread> threads;
    std::vector<long long> wins(NUM_COMBO, 0), draws(NUM_COMBO, 0);

    for (unsigned t = 0; t < hw; t++) {
        threads.emplace_back([&, t]() {
            for (int i = t; i < NUM_COMBO; i += (int)hw) {
                long long w = 0, d = 0;
                const ComboEntry& ei = g_combos[i];
                for (int j = 0; j < NUM_COMBO; j++) {
                    if ((ei.mask & g_combos[j].mask) != 0) continue;  // 重叠
                    int cmp = HandEvaluator::compare(ei.hr, g_combos[j].hr);
                    if (cmp > 0) w++;
                    else if (cmp == 0) d++;
                }
                wins[i] = w;
                draws[i] = d;
            }
        });
    }
    for (auto& th : threads) th.join();

    for (int i = 0; i < NUM_COMBO; i++) {
        table[i] = (float)(wins[i] + 0.5 * draws[i]) / (float)NUM_OPPONENT;
    }

    FILE* f = nullptr;
#ifdef _MSC_VER
    fopen_s(&f, path.c_str(), "wb");
#else
    f = fopen(path.c_str(), "wb");
#endif
    if (!f) return false;
    fwrite(table.data(), sizeof(float), table.size(), f);
    fclose(f);

    if (verbose) {
        std::printf("胜率表已保存: %s (%.1f KB)\n", path.c_str(),
                    (double)(table.size() * sizeof(float)) / 1024.0);
    }
    return true;
}

bool AIPlayer::loadWinRateTable(const std::string& path) {
    FILE* f = nullptr;
#ifdef _MSC_VER
    fopen_s(&f, path.c_str(), "rb");
#else
    f = fopen(path.c_str(), "rb");
#endif
    if (!f) return false;
    std::vector<float> table(NUM_COMBO);
    size_t read = fread(table.data(), sizeof(float), NUM_COMBO, f);
    fclose(f);
    if (read != NUM_COMBO) return false;
    s_winrate = std::move(table);
    s_loaded = true;
    buildComboTable();
    return true;
}

float AIPlayer::winRateOf(const Card& a, const Card& b, const Card& c) {
    if (!s_loaded) return -1.0f;
    int ids[3] = { cardId(a), cardId(b), cardId(c) };
    std::sort(ids, ids + 3);
    return s_winrate[combIndex(ids[0], ids[1], ids[2])];
}

// 胜率表: 赢过所有对手 ≈ 每个对手独立: p^opponents
float AIPlayer::groupScore(const Card* hand, int idx0, int idx1, int idx2, int opponents) {
    float wr = winRateOf(hand[idx0], hand[idx1], hand[idx2]);
    if (wr < 0.f) wr = 0.f;   // 表未加载 -> 记为 0 分
    return std::pow(wr, (float)opponents);
}

// 手牌下标 0..8, 头道=前3个, 中道=中间3个, 尾道=最后3个
template <typename Fn>
void AIPlayer::forEachSplit(const Card* /*hand*/, Fn fn) {   // hand 未使用
    int order[9];
    // 枚举头道 =84)
    for (int a = 0; a < 9; a++) {
        for (int b = a + 1; b < 9; b++) {
            for (int c = b + 1; c < 9; c++) {
                int line0[3] = { a, b, c };
                // 剩余 6 张
                int rest[6]; int n = 0;
                for (int i = 0; i < 9; i++) {
                    if (i != a && i != b && i != c) rest[n++] = i;
                }
                // 枚举中道 =20)
                for (int d = 0; d < 6; d++) {
                    for (int e = d + 1; e < 6; e++) {
                        for (int f = e + 1; f < 6; f++) {
                            int line1[3] = { rest[d], rest[e], rest[f] };
                            int line2[3]; int m = 0;
                            for (int i = 0; i < 6; i++) {
                                if (i != d && i != e && i != f) line2[m++] = rest[i];
                            }
                            // 头道顺序固定, 中/尾按升序; 三道之间顺序就是头/中/尾
                            // 为了让排序稳定, 头道取升序即可
                            for (int k = 0; k < 3; k++) {
                                order[k] = line0[k];
                                order[3 + k] = line1[k];
                                order[6 + k] = line2[k];
                            }
                            fn(line0, line1, line2, order);
                        }
                    }
                }
            }
        }
    }
}

float AIPlayer::bestGreedy(const Card* hand, int opponents, int order[9]) {
    float bestScore = -1.0f;
    int bestOrder[9] = {0,1,2,3,4,5,6,7,8};
    forEachSplit(hand, [&](const int* l0, const int* l1, const int* l2, const int* ord) {
        float s = groupScore(hand, l0[0], l0[1], l0[2], opponents)
                + groupScore(hand, l1[0], l1[1], l1[2], opponents)
                + groupScore(hand, l2[0], l2[1], l2[2], opponents);
        if (s > bestScore) {
            bestScore = s;
            for (int i = 0; i < 9; i++) bestOrder[i] = ord[i];
        }
    });
    for (int i = 0; i < 9; i++) order[i] = bestOrder[i];
    return bestScore;
}

// 候选 = 贪心 top-K; 对手模型 = 贪心
// 模拟次数 = 难度档位相关
void AIPlayer::monteCarloChoose(const Card* hand, int opponents, int order[9], int sims, int topK) {
    std::mt19937 rng((unsigned)std::random_device{}());

    // 1. 收集贪心评分并排序, 取 top-K
    struct Cand { float score; int ord[9]; };
    std::vector<Cand> cands;
    cands.reserve(SPLIT_COUNT);
    forEachSplit(hand, [&](const int* l0, const int* l1, const int* l2, const int* ord) {
        float s = groupScore(hand, l0[0], l0[1], l0[2], opponents)
                + groupScore(hand, l1[0], l1[1], l1[2], opponents)
                + groupScore(hand, l2[0], l2[1], l2[2], opponents);
        Cand c; c.score = s;
        for (int i = 0; i < 9; i++) c.ord[i] = ord[i];
        cands.push_back(c);
    });
    std::sort(cands.begin(), cands.end(), [](const Cand& x, const Cand& y) { return x.score > y.score; });
    if ((int)cands.size() > topK) cands.resize(topK);

    // 2. 对每个候选模拟
    // 剩余牌 = 54 - 9 - opponents*9
    int deckIds[45];
    bool used[54] = { false };
    for (int i = 0; i < 9; i++) used[cardId(hand[i])] = true;

    std::vector<float> avg(cands.size(), 0.0f);
    for (int s = 0; s < sims; s++) {
        // 抽对手的牌
        int n = 0;
        for (int i = 0; i < 54; i++) if (!used[i]) deckIds[n++] = i;
        // 洗牌取前 opponents*9
        std::shuffle(deckIds, deckIds + n, rng);
        std::vector<Card> oppHands[6];   // 最多 6 个对手? opponents<=5
        for (int o = 0; o < opponents; o++) {
            for (int k = 0; k < 9; k++) {
                oppHands[o].push_back(cardFromId(deckIds[o * 9 + k]));
            }
        }
        // 每个对手贪心分组
        int oppOrder[6][9];
        for (int o = 0; o < opponents; o++) {
            bestGreedy(oppHands[o].data(), opponents - 1, oppOrder[o]);
        }
        // 对每个候选, 三道逐道比较
        for (size_t ci = 0; ci < cands.size(); ci++) {
            float won = 0.0f;
            for (int line = 0; line < 3; line++) {
                std::vector<Card> my3 = {
                    hand[cands[ci].ord[line*3]],
                    hand[cands[ci].ord[line*3+1]],
                    hand[cands[ci].ord[line*3+2]]
                };
                HandResult myHr = HandEvaluator::evaluate(my3);
                bool beatAll = true;
                for (int o = 0; o < opponents; o++) {
                    const int* oo = oppOrder[o];
                    std::vector<Card> op3 = {
                        oppHands[o][oo[line*3]],
                        oppHands[o][oo[line*3+1]],
                        oppHands[o][oo[line*3+2]]
                    };
                    HandResult opHr = HandEvaluator::evaluate(op3);
                    int cmp = HandEvaluator::compare(myHr, opHr);
                    if (cmp < 0) { beatAll = false; break; }
                    if (cmp == 0) { beatAll = false; break; }  // 平局不算赢
                }
                if (beatAll) won += 1.0f;
            }
            avg[ci] += won;
        }
    }
    for (size_t i = 0; i < avg.size(); i++) avg[i] /= (float)sims;

    // 3. 选期望赢池最高的
    int best = 0;
    for (size_t i = 1; i < avg.size(); i++) {
        if (avg[i] > avg[best]) best = (int)i;
    }
    for (int i = 0; i < 9; i++) order[i] = cands[best].ord[i];
}

void AIPlayer::decideOrder(const Card* hand, int playerCount, Difficulty diff, int order[9]) {
    // 环境变量覆盖: 仅当设置了 SDQ_AI_DIFFICULTY 时生效
    if (userProfileSet()) diff = s_difficulty;
    int opponents = playerCount - 1;
    if (opponents < 1) opponents = 1;

    if (diff == Difficulty::Random) {
        // 随机排列 0..8
        std::mt19937 rng((unsigned)std::random_device{}());
        for (int i = 0; i < 9; i++) order[i] = i;
        std::shuffle(order, order + 9, rng);
        return;
    }

    if (diff == Difficulty::Greedy) {
        bestGreedy(hand, opponents, order);
        return;
    }

    // MonteCarlo: 模拟次数按对手数调整
    int sims = 20;
    if (opponents >= 3) sims = 100;
    if (opponents >= 5) sims = 60;
    monteCarloChoose(hand, opponents, order, sims, 30);
}

// 人性化扩展: 风格 + 失误率

float AIPlayer::styleScore(const float w[3], Style style) {
    float sum = w[0] + w[1] + w[2];
    float mx = std::max(w[0], std::max(w[1], w[2]));
    float mn = std::min(w[0], std::min(w[1], w[2]));
    switch (style) {
        case Style::Aggressive:   return sum + 0.5f * mx;  // 偏爱一组超强
        case Style::Conservative: return sum + 0.5f * mn;  // 偏爱三组均衡
        default:                  return sum;              // 均衡
    }
}

int AIPlayer::humanPick(const std::vector<float>& scores, int topK, float noise) {
    std::vector<int> idx(scores.size());
    for (size_t i = 0; i < scores.size(); i++) idx[i] = (int)i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) { return scores[a] > scores[b]; });
    if (noise <= 0.0f) return idx[0];
    std::mt19937 rng((unsigned)std::random_device{}());
    float roll = (float)rng() / (float)rng.max();
    if (roll < noise) {
        int k = (int)idx.size();
        if (topK < k) k = topK;
        if (k <= 1) return idx[0];
        return idx[(int)((float)rng() / (float)rng.max() * k)];
    }
    return idx[0];
}

int AIPlayer::bestGreedyStyled(const Card* hand, int opponents, Style style, float noise, int order[9]) {
    struct Cand { float score; int ord[9]; };
    std::vector<Cand> cands;
    cands.reserve(SPLIT_COUNT);
    forEachSplit(hand, [&](const int* l0, const int* l1, const int* l2, const int* ord) {
        float w[3];
        w[0] = groupScore(hand, l0[0], l0[1], l0[2], opponents);
        w[1] = groupScore(hand, l1[0], l1[1], l1[2], opponents);
        w[2] = groupScore(hand, l2[0], l2[1], l2[2], opponents);
        Cand c; c.score = styleScore(w, style);
        for (int i = 0; i < 9; i++) c.ord[i] = ord[i];
        cands.push_back(c);
    });
    std::vector<float> scores;
    scores.reserve(cands.size());
    for (auto& c : cands) scores.push_back(c.score);
    int pick = humanPick(scores, 5, noise);
    for (int i = 0; i < 9; i++) order[i] = cands[pick].ord[i];
    return pick;
}

void AIPlayer::monteCarloStyled(const Card* hand, int opponents, int order[9],
                                int sims, int topK, Style style, float noise) {
    std::mt19937 rng((unsigned)std::random_device{}());
    struct Cand { float score; int ord[9]; };
    std::vector<Cand> cands;
    cands.reserve(SPLIT_COUNT);
    forEachSplit(hand, [&](const int* l0, const int* l1, const int* l2, const int* ord) {
        float w[3];
        w[0] = groupScore(hand, l0[0], l0[1], l0[2], opponents);
        w[1] = groupScore(hand, l1[0], l1[1], l1[2], opponents);
        w[2] = groupScore(hand, l2[0], l2[1], l2[2], opponents);
        Cand c; c.score = styleScore(w, style);
        for (int i = 0; i < 9; i++) c.ord[i] = ord[i];
        cands.push_back(c);
    });
    std::sort(cands.begin(), cands.end(), [](const Cand& x, const Cand& y) { return x.score > y.score; });
    if ((int)cands.size() > topK) cands.resize(topK);

    bool used[54] = { false };
    for (int i = 0; i < 9; i++) used[cardId(hand[i])] = true;
    int deckIds[45];
    int n = 0;
    for (int i = 0; i < 54; i++) if (!used[i]) deckIds[n++] = i;

    std::vector<float> avg(cands.size(), 0.0f);
    for (int s = 0; s < sims; s++) {
        std::shuffle(deckIds, deckIds + n, rng);
        std::vector<Card> oppHands[6];
        for (int o = 0; o < opponents; o++) {
            for (int k = 0; k < 9; k++) oppHands[o].push_back(cardFromId(deckIds[o * 9 + k]));
        }
        int oppOrder[6][9];
        for (int o = 0; o < opponents; o++) {
            bestGreedy(oppHands[o].data(), opponents - 1, oppOrder[o]);
        }
        for (size_t ci = 0; ci < cands.size(); ci++) {
            float won = 0.0f;
            for (int line = 0; line < 3; line++) {
                std::vector<Card> my3 = { hand[cands[ci].ord[line*3]], hand[cands[ci].ord[line*3+1]], hand[cands[ci].ord[line*3+2]] };
                HandResult myHr = HandEvaluator::evaluate(my3);
                bool beatAll = true;
                for (int o = 0; o < opponents; o++) {
                    const int* oo = oppOrder[o];
                    std::vector<Card> op3 = { oppHands[o][oo[line*3]], oppHands[o][oo[line*3+1]], oppHands[o][oo[line*3+2]] };
                    HandResult opHr = HandEvaluator::evaluate(op3);
                    if (HandEvaluator::compare(myHr, opHr) <= 0) { beatAll = false; break; }
                }
                if (beatAll) won += 1.0f;
            }
            avg[ci] += won;
        }
    }
    for (size_t i = 0; i < avg.size(); i++) avg[i] /= (float)sims;
    int pick = humanPick(avg, 3, noise);
    for (int i = 0; i < 9; i++) order[i] = cands[pick].ord[i];
}

void AIPlayer::decideOrderStyled(const Card* hand, int playerCount,
                                 Difficulty diff, Style style, float noise,
                                 int order[9]) {
    if (userProfileSet()) {
        diff  = s_difficulty;
        style = s_style;
        noise = s_noise;
    }
    int opponents = playerCount - 1;
    if (opponents < 1) opponents = 1;
    if (diff == Difficulty::Random) {
        std::mt19937 rng((unsigned)std::random_device{}());
        for (int i = 0; i < 9; i++) order[i] = i;
        std::shuffle(order, order + 9, rng);
        return;
    }
    if (diff == Difficulty::Greedy) {
        bestGreedyStyled(hand, opponents, style, noise, order);
        return;
    }
    int sims = 20;
    if (opponents >= 3) sims = 100;
    if (opponents >= 5) sims = 60;
    monteCarloStyled(hand, opponents, order, sims, 30, style, noise);
}

int AIPlayer::diversityOf(const Card* hand, int playerCount,
                          Difficulty diff, Style style, float noise,
                          int trials) {
    std::vector<unsigned long long> seen;
    for (int t = 0; t < trials; t++) {
        int order[9];
        decideOrderStyled(hand, playerCount, diff, style, noise, order);
        unsigned long long key = 0;
        for (int i = 0; i < 9; i++) key = key * 9 + (unsigned long long)order[i];
        bool dup = false;
        for (size_t i = 0; i < seen.size(); i++) {
            if (seen[i] == key) { dup = true; break; }
        }
        if (!dup) seen.push_back(key);
    }
    return (int)seen.size();
}

void AIPlayer::setProfile(Difficulty d, Style s, float noise) {
    s_difficulty = d;
    s_style = s;
    setNoise(noise);
    s_profileSet = true;
}

void AIPlayer::setNoise(float n) {
    if (n < 0.0f) n = 0.0f;
    if (n > 1.0f) n = 1.0f;
    s_noise = n;
    s_profileSet = true;
}

AIPlayer::Difficulty AIPlayer::difficulty() { return s_difficulty; }

const char* AIPlayer::difficultyName(Difficulty d) {
    switch (d) {
        case Difficulty::Random:     return "简单";
        case Difficulty::MonteCarlo: return "困难";
        default:                     return "中等";
    }
}

// 键名 -> 难度
bool AIPlayer::difficultyFromKey(const std::string& key, Difficulty& out) {
    std::string k;
    for (size_t i = 0; i < key.size(); i++) {
        char c = key[i];
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        k.push_back(c);
    }
    if (k == "random" || k == "easy" || k == "0") { out = Difficulty::Random; return true; }
    if (k == "greedy" || k == "normal" || k == "1") { out = Difficulty::Greedy; return true; }
    if (k == "montecarlo" || k == "mc" || k == "hard" || k == "2") { out = Difficulty::MonteCarlo; return true; }
    return false;
}

// 用当前配置决策: 界面层把 decideOrderStyled 换成这一个即可
void AIPlayer::decideOrderAuto(const Card* hand, int playerCount, int order[9]) {
    decideOrderStyled(hand, playerCount, s_difficulty, s_style, s_noise, order);
}

// 环境变量 只在首次调用时读一次
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)   // std::getenv: 仅用于可选的调试开关, 非生产路径
#endif
bool AIPlayer::applyEnvConfigOnce() {
    const char* d  = std::getenv("SDQ_AI_DIFFICULTY");
    const char* st = std::getenv("SDQ_AI_STYLE");
    const char* nz = std::getenv("SDQ_AI_NOISE");
    if (!d && !st && !nz) return false;
    if (d) {
        Difficulty tmp;
        if (difficultyFromKey(d, tmp)) s_difficulty = tmp;
    }
    if (st) {
        std::string k(st);
        for (size_t i = 0; i < k.size(); i++) {
            if (k[i] >= 'A' && k[i] <= 'Z') k[i] = (char)(k[i] - 'A' + 'a');
        }
        if (k == "aggressive") s_style = Style::Aggressive;
        else if (k == "conservative") s_style = Style::Conservative;
        else s_style = Style::Balanced;
    }
    if (nz) setNoise((float)std::atof(nz));
    s_profileSet = true;
    return true;
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

bool AIPlayer::userProfileSet() {
    static const bool s_envApplied = applyEnvConfigOnce();  // 首次调用时落地环境变量
    (void)s_envApplied;
    return s_profileSet;
}
