#include "core/NameGen.h"
#include <random>

namespace {
const char* PRE[] = {
    "孤", "疾", "冷", "狂", "影", "铁", "夜", "风", "雷", "星",
    "龙", "虎", "苍", "云", "火", "霜", "醉", "玄", "赤", "幽"
};
const char* SUF[] = {
    "影", "刃", "客", "王", "鹰", "狼", "侠", "神", "灵", "锋",
    "尘", "澜", "月", "歌", "雪", "鸿", "辰", "瑶", "冥", "羽"
};
const int PRE_N = static_cast<int>(sizeof(PRE) / sizeof(PRE[0]));
const int SUF_N = static_cast<int>(sizeof(SUF) / sizeof(SUF[0]));

std::mt19937& rng() {
    static std::mt19937 g(std::random_device{}());
    return g;
}
}

std::string makeNickname() {
    std::uniform_int_distribution<int> dp(0, PRE_N - 1);
    std::uniform_int_distribution<int> ds(0, SUF_N - 1);
    return std::string(PRE[dp(rng())]) + SUF[ds(rng())];
}

std::string makeUniqueNickname(const std::string* used, int usedCount) {
    for (int t = 0; t < 64; t++) {           // 随机重试
        std::string nm = makeNickname();
        bool dup = false;
        for (int i = 0; i < usedCount; i++) {
            if (used[i] == nm) { dup = true; break; }
        }
        if (!dup) return nm;
    }
    // 兜底: 组合已用尽时追加序号
    std::string base = makeNickname();
    for (int k = 2; k < 9999; k++) {
        std::string nm = base + std::to_string(k);
        bool dup = false;
        for (int i = 0; i < usedCount; i++) {
            if (used[i] == nm) { dup = true; break; }
        }
        if (!dup) return nm;
    }
    return base + "X";
}
