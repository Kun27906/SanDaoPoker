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
// 形容词字表: 与中心词同样用"前缀字 + 后缀字"组合
const char* ADJ_PRE[] = {
    "孤", "冷", "狂", "静", "幽", "沉", "灵", "疾", "烈", "玄",
    "苍", "赤", "醉", "逍", "豪", "狡", "温", "凛", "傲", "勇"
};
const char* ADJ_SUF[] = {
    "傲", "冽", "逸", "默", "猛", "锐", "勇", "烈", "静", "灵",
    "巧", "稳", "韧", "疾", "雅", "秀", "冷", "野", "狂", "然"
};
const int PRE_N = static_cast<int>(sizeof(PRE) / sizeof(PRE[0]));
const int SUF_N = static_cast<int>(sizeof(SUF) / sizeof(SUF[0]));
const int ADJ_PRE_N = static_cast<int>(sizeof(ADJ_PRE) / sizeof(ADJ_PRE[0]));
const int ADJ_SUF_N = static_cast<int>(sizeof(ADJ_SUF) / sizeof(ADJ_SUF[0]));

std::mt19937& rng() {
    static std::mt19937 g(std::random_device{}());
    return g;
}
}

// 生成昵称: 形容词 + 中心词。
// "的"字按 40% 概率省略 -> 四字与五字两种 id 并存。
std::string makeNickname() {
    std::uniform_int_distribution<int> dp(0, PRE_N - 1);
    std::uniform_int_distribution<int> ds(0, SUF_N - 1);
    std::uniform_int_distribution<int> ap(0, ADJ_PRE_N - 1);
    std::uniform_int_distribution<int> as(0, ADJ_SUF_N - 1);
    std::uniform_int_distribution<int> dmid(0, 99);   // 40% 不加"的"
    const std::string adj = std::string(ADJ_PRE[ap(rng())]) + ADJ_SUF[as(rng())];
    const std::string ctr = std::string(PRE[dp(rng())]) + SUF[ds(rng())];
    return (dmid(rng()) < 40) ? (adj + ctr) : (adj + "的" + ctr);
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
