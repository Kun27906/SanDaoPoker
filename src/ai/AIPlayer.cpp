#include "ai/AIPlayer.h"
#include "core/HandEvaluator.h"
#include "core/Deck.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <numeric>
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

constexpr int NUM_COMBO = 24804;      // 54 选 3
constexpr int NUM_OPPONENT = 20825;   // 对手三张组合数
constexpr int SPLIT_COUNT = 1680;     // 九张牌分三道

struct ComboEntry {
    int id[3];
    unsigned long long mask;
    HandResult hr;
};

static std::vector<ComboEntry> g_combos;
static std::unordered_map<unsigned long long, int> g_comboIndex;

// 三张牌编号打包为查找键
static unsigned long long comboKey(int a, int b, int c) {
    return (static_cast<unsigned long long>(a) << 12)
         | (static_cast<unsigned long long>(b) << 6)
         | static_cast<unsigned long long>(c);
}

// 转小写
static std::string lower(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return s;
}

// 与 Deck 生成顺序一致, 小王 52 大王 53
int AIPlayer::cardId(const Card& c) {
    if (c.isSmallJoker()) return 52;
    if (c.isBigJoker()) return 53;
    return c.getSuitValue() * 13 + (c.getRankValue() - 2);
}

int AIPlayer::combIndex(int a, int b, int c) {
    const auto it = g_comboIndex.find(comboKey(a, b, c));
    return (it != g_comboIndex.end()) ? it->second : 0;
}

static Card cardFromId(int id) {
    if (id == 52) return Card(Rank::SmallJoker);
    if (id == 53) return Card(Rank::BigJoker);
    return Card(static_cast<Suit>(id / 13), static_cast<Rank>(id % 13 + 2));
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
                g_comboIndex[comboKey(a, b, c)] = idx;
                idx++;
            }
        }
    }
}

// 逐组合精确统计胜平负, 平局记半胜
bool AIPlayer::generateWinRateTable(const std::string& path, bool verbose) {
    buildComboTable();
    std::vector<float> table(NUM_COMBO, 0.0f);

    if (verbose) {
        std::printf("生成胜率表: %d 个组合, 每个 vs %d 个对手组合...\n", NUM_COMBO, NUM_OPPONENT);
        std::fflush(stdout);
    }

    const unsigned hw = std::clamp(std::thread::hardware_concurrency(), 2u, 8u);
    std::vector<std::thread> threads;
    std::vector<long long> wins(NUM_COMBO, 0), draws(NUM_COMBO, 0);

    for (unsigned t = 0; t < hw; t++) {
        threads.emplace_back([&, t]() {
            for (int i = static_cast<int>(t); i < NUM_COMBO; i += static_cast<int>(hw)) {
                long long w = 0, d = 0;
                const ComboEntry& ei = g_combos[i];
                for (int j = 0; j < NUM_COMBO; j++) {
                    if ((ei.mask & g_combos[j].mask) != 0) continue;
                    const int cmp = HandEvaluator::compare(ei.hr, g_combos[j].hr);
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
        table[i] = static_cast<float>(wins[i] + 0.5 * draws[i]) / static_cast<float>(NUM_OPPONENT);
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
                    static_cast<double>(table.size() * sizeof(float)) / 1024.0);
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
    const size_t read = fread(table.data(), sizeof(float), NUM_COMBO, f);
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

// 单组赢过全场概率近似为各对手独立
float AIPlayer::groupScore(const Card* hand, int idx0, int idx1, int idx2, int opponents) {
    float wr = winRateOf(hand[idx0], hand[idx1], hand[idx2]);
    if (wr < 0.f) wr = 0.f;
    return std::pow(wr, static_cast<float>(opponents));
}

// 枚举九张牌的全部分道, 回调收到 头道 中道 尾道 与合并后的 order
template <typename Fn>
void AIPlayer::forEachSplit(Fn fn) {
    int order[9];
    for (int a = 0; a < 9; a++) {
        for (int b = a + 1; b < 9; b++) {
            for (int c = b + 1; c < 9; c++) {
                const int line0[3] = { a, b, c };
                int rest[6]; int n = 0;
                for (int i = 0; i < 9; i++) if (i != a && i != b && i != c) rest[n++] = i;
                for (int d = 0; d < 6; d++) {
                    for (int e = d + 1; e < 6; e++) {
                        for (int f = e + 1; f < 6; f++) {
                            const int line1[3] = { rest[d], rest[e], rest[f] };
                            int line2[3]; int m = 0;
                            for (int i = 0; i < 6; i++) if (i != d && i != e && i != f) line2[m++] = rest[i];
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
    int bestOrder[9] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
    forEachSplit([&](const int* l0, const int* l1, const int* l2, const int* ord) {
        const float s = groupScore(hand, l0[0], l0[1], l0[2], opponents)
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

// 候选取贪心前 K, 蒙特卡洛模拟对手后选期望赢池最高
void AIPlayer::monteCarloChoose(const Card* hand, int opponents, int order[9], int sims, int topK) {
    std::mt19937 rng(std::random_device{}());

    struct Cand { float score; int ord[9]; float avg = 0.0f; };
    std::vector<Cand> cands;
    cands.reserve(SPLIT_COUNT);
    forEachSplit([&](const int* l0, const int* l1, const int* l2, const int* ord) {
        Cand c;
        c.score = groupScore(hand, l0[0], l0[1], l0[2], opponents)
                + groupScore(hand, l1[0], l1[1], l1[2], opponents)
                + groupScore(hand, l2[0], l2[1], l2[2], opponents);
        for (int i = 0; i < 9; i++) c.ord[i] = ord[i];
        cands.push_back(c);
    });
    std::sort(cands.begin(), cands.end(), [](const Cand& x, const Cand& y) { return x.score > y.score; });
    if (static_cast<int>(cands.size()) > topK) cands.resize(topK);

    int deckIds[45];
    bool used[54] = { false };
    for (int i = 0; i < 9; i++) used[cardId(hand[i])] = true;

    for (int s = 0; s < sims; s++) {
        int n = 0;
        for (int i = 0; i < 54; i++) if (!used[i]) deckIds[n++] = i;
        std::shuffle(deckIds, deckIds + n, rng);

        std::vector<Card> oppHands[6];
        for (int o = 0; o < opponents; o++) {
            for (int k = 0; k < 9; k++) oppHands[o].push_back(cardFromId(deckIds[o * 9 + k]));
        }
        int oppOrder[6][9];
        for (int o = 0; o < opponents; o++) bestGreedy(oppHands[o].data(), opponents - 1, oppOrder[o]);

        for (Cand& c : cands) {
            float won = 0.0f;
            for (int line = 0; line < 3; line++) {
                std::vector<Card> my3 = {
                    hand[c.ord[line * 3]], hand[c.ord[line * 3 + 1]], hand[c.ord[line * 3 + 2]]
                };
                const HandResult myHr = HandEvaluator::evaluate(my3);
                bool beatAll = true;
                for (int o = 0; o < opponents; o++) {
                    const int* oo = oppOrder[o];
                    std::vector<Card> op3 = {
                        oppHands[o][oo[line * 3]], oppHands[o][oo[line * 3 + 1]], oppHands[o][oo[line * 3 + 2]]
                    };
                    if (HandEvaluator::compare(myHr, HandEvaluator::evaluate(op3)) <= 0) { beatAll = false; break; }
                }
                if (beatAll) won += 1.0f;
            }
            c.avg += won;
        }
    }
    for (Cand& c : cands) c.avg /= static_cast<float>(sims);

    const auto best = std::max_element(cands.begin(), cands.end(),
        [](const Cand& x, const Cand& y) { return x.avg < y.avg; });
    for (int i = 0; i < 9; i++) order[i] = best->ord[i];
}

void AIPlayer::decideOrder(const Card* hand, int playerCount, Difficulty diff, int order[9]) {
    if (userProfileSet()) diff = s_difficulty;
    const int opponents = std::max(playerCount - 1, 1);

    if (diff == Difficulty::Random) {
        std::mt19937 rng(std::random_device{}());
        for (int i = 0; i < 9; i++) order[i] = i;
        std::shuffle(order, order + 9, rng);
        return;
    }
    if (diff == Difficulty::Greedy) {
        bestGreedy(hand, opponents, order);
        return;
    }

    int sims = 20;
    if (opponents >= 3) sims = 100;
    if (opponents >= 5) sims = 60;
    monteCarloChoose(hand, opponents, order, sims, 30);
}

// 风格加分: 激进偏爱强组, 稳健偏爱均衡
float AIPlayer::styleScore(const float w[3], Style style) {
    const float sum = w[0] + w[1] + w[2];
    const float mx = std::max({ w[0], w[1], w[2] });
    const float mn = std::min({ w[0], w[1], w[2] });
    switch (style) {
        case Style::Aggressive:   return sum + 0.5f * mx;
        case Style::Conservative: return sum + 0.5f * mn;
        default:                  return sum;
    }
}

// 按失误率在前 K 内随机挑选, 否则取最优
int AIPlayer::humanPick(const std::vector<float>& scores, int topK, float noise) {
    std::vector<int> idx(scores.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](int a, int b) { return scores[a] > scores[b]; });
    if (noise <= 0.0f) return idx[0];
    std::mt19937 rng(std::random_device{}());
    const float roll = static_cast<float>(rng()) / static_cast<float>(rng.max());
    if (roll < noise) {
        int k = static_cast<int>(idx.size());
        if (topK < k) k = topK;
        if (k <= 1) return idx[0];
        return idx[static_cast<int>(static_cast<float>(rng()) / static_cast<float>(rng.max()) * k)];
    }
    return idx[0];
}

int AIPlayer::bestGreedyStyled(const Card* hand, int opponents, Style style, float noise, int order[9]) {
    struct Cand { float score; int ord[9]; };
    std::vector<Cand> cands;
    cands.reserve(SPLIT_COUNT);
    forEachSplit([&](const int* l0, const int* l1, const int* l2, const int* ord) {
        const float w[3] = {
            groupScore(hand, l0[0], l0[1], l0[2], opponents),
            groupScore(hand, l1[0], l1[1], l1[2], opponents),
            groupScore(hand, l2[0], l2[1], l2[2], opponents)
        };
        Cand c;
        c.score = styleScore(w, style);
        for (int i = 0; i < 9; i++) c.ord[i] = ord[i];
        cands.push_back(c);
    });
    std::vector<float> scores;
    scores.reserve(cands.size());
    for (const Cand& c : cands) scores.push_back(c.score);
    const int pick = humanPick(scores, 5, noise);
    for (int i = 0; i < 9; i++) order[i] = cands[pick].ord[i];
    return pick;
}

void AIPlayer::monteCarloStyled(const Card* hand, int opponents, int order[9],
                                int sims, int topK, Style style, float noise) {
    std::mt19937 rng(std::random_device{}());

    struct Cand { float score; int ord[9]; float avg = 0.0f; };
    std::vector<Cand> cands;
    cands.reserve(SPLIT_COUNT);
    forEachSplit([&](const int* l0, const int* l1, const int* l2, const int* ord) {
        const float w[3] = {
            groupScore(hand, l0[0], l0[1], l0[2], opponents),
            groupScore(hand, l1[0], l1[1], l1[2], opponents),
            groupScore(hand, l2[0], l2[1], l2[2], opponents)
        };
        Cand c;
        c.score = styleScore(w, style);
        for (int i = 0; i < 9; i++) c.ord[i] = ord[i];
        cands.push_back(c);
    });
    std::sort(cands.begin(), cands.end(), [](const Cand& x, const Cand& y) { return x.score > y.score; });
    if (static_cast<int>(cands.size()) > topK) cands.resize(topK);

    int deckIds[45];
    bool used[54] = { false };
    for (int i = 0; i < 9; i++) used[cardId(hand[i])] = true;
    int n = 0;
    for (int i = 0; i < 54; i++) if (!used[i]) deckIds[n++] = i;

    for (int s = 0; s < sims; s++) {
        std::shuffle(deckIds, deckIds + n, rng);
        std::vector<Card> oppHands[6];
        for (int o = 0; o < opponents; o++) {
            for (int k = 0; k < 9; k++) oppHands[o].push_back(cardFromId(deckIds[o * 9 + k]));
        }
        int oppOrder[6][9];
        for (int o = 0; o < opponents; o++) bestGreedy(oppHands[o].data(), opponents - 1, oppOrder[o]);

        for (Cand& c : cands) {
            float won = 0.0f;
            for (int line = 0; line < 3; line++) {
                std::vector<Card> my3 = {
                    hand[c.ord[line * 3]], hand[c.ord[line * 3 + 1]], hand[c.ord[line * 3 + 2]]
                };
                const HandResult myHr = HandEvaluator::evaluate(my3);
                bool beatAll = true;
                for (int o = 0; o < opponents; o++) {
                    const int* oo = oppOrder[o];
                    std::vector<Card> op3 = {
                        oppHands[o][oo[line * 3]], oppHands[o][oo[line * 3 + 1]], oppHands[o][oo[line * 3 + 2]]
                    };
                    if (HandEvaluator::compare(myHr, HandEvaluator::evaluate(op3)) <= 0) { beatAll = false; break; }
                }
                if (beatAll) won += 1.0f;
            }
            c.avg += won;
        }
    }
    for (Cand& c : cands) c.avg /= static_cast<float>(sims);

    std::vector<float> avg;
    avg.reserve(cands.size());
    for (const Cand& c : cands) avg.push_back(c.avg);
    const int pick = humanPick(avg, 3, noise);
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
    const int opponents = std::max(playerCount - 1, 1);

    if (diff == Difficulty::Random) {
        std::mt19937 rng(std::random_device{}());
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

// 同一手牌多次决策, 返回不同方案数
int AIPlayer::diversityOf(const Card* hand, int playerCount,
                          Difficulty diff, Style style, float noise,
                          int trials) {
    std::vector<unsigned long long> seen;
    for (int t = 0; t < trials; t++) {
        int order[9];
        decideOrderStyled(hand, playerCount, diff, style, noise, order);
        unsigned long long key = 0;
        for (int i = 0; i < 9; i++) key = key * 9 + static_cast<unsigned long long>(order[i]);
        if (std::find(seen.begin(), seen.end(), key) == seen.end()) seen.push_back(key);
    }
    return static_cast<int>(seen.size());
}

void AIPlayer::setProfile(Difficulty d, Style s, float noise) {
    s_difficulty = d;
    s_style = s;
    setNoise(noise);
    s_profileSet = true;
}

void AIPlayer::setNoise(float n) {
    s_noise = std::clamp(n, 0.0f, 1.0f);
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

bool AIPlayer::difficultyFromKey(const std::string& key, Difficulty& out) {
    const std::string k = lower(key);
    if (k == "random" || k == "easy" || k == "0") { out = Difficulty::Random; return true; }
    if (k == "greedy" || k == "normal" || k == "1") { out = Difficulty::Greedy; return true; }
    if (k == "montecarlo" || k == "mc" || k == "hard" || k == "2") { out = Difficulty::MonteCarlo; return true; }
    return false;
}

void AIPlayer::decideOrderAuto(const Card* hand, int playerCount, int order[9]) {
    decideOrderStyled(hand, playerCount, s_difficulty, s_style, s_noise, order);
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)   // getenv 仅作可选调试开关
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
        const std::string k = lower(st);
        if (k == "aggressive") s_style = Style::Aggressive;
        else if (k == "conservative") s_style = Style::Conservative;
        else s_style = Style::Balanced;
    }
    if (nz) setNoise(static_cast<float>(std::atof(nz)));
    s_profileSet = true;
    return true;
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif

bool AIPlayer::userProfileSet() {
    static const bool envApplied = applyEnvConfigOnce();
    (void)envApplied;
    return s_profileSet;
}
