#include "core/Deck.h"
#include <algorithm>
#include <random>
#include <stdexcept>

// ====== 构造函数：生成一副完整54张牌 ======

Deck::Deck() {
    reset();
}

// ====== 重置为一副新牌 ======

void Deck::reset() {
    cards_.clear();
    cards_.reserve(54);  // 预分配空间，避免多次扩容

    // 52张普通牌：4种花色 × 13个点数
    for (int s = 0; s < 4; s++) {
        for (int r = 2; r <= 14; r++) {
            cards_.emplace_back(
                static_cast<Suit>(s),
                static_cast<Rank>(r)
            );
        }
    }

    // 2张王
    cards_.emplace_back(Rank::SmallJoker);
    cards_.emplace_back(Rank::BigJoker);
}

// ====== 洗牌 ======
// 用 Mersenne Twister (mt19937) 随机数引擎
// seed=0 → 用随机设备生成种子（每次不同）
// seed!=0 → 用固定种子（方便测试复现）

void Deck::shuffle(unsigned seed) {
    // 初始化随机数引擎
    std::mt19937 rng;

    if (seed == 0) {
        // 用硬件随机设备生成种子（每次洗牌结果不同）
        std::random_device rd;
        rng.seed(rd());
    } else {
        // 用固定种子（方便调试：每次结果一样）
        rng.seed(seed);
    }

    // Fisher-Yates 洗牌算法（std::shuffle 内部就是这个）
    std::shuffle(cards_.begin(), cards_.end(), rng);
}

// ====== 发牌 ======
// 给 numPlayers 个人每人发 9 张
// 逆时针 = 从第0个人开始，轮流发牌，每人每次1张，发9轮
// 总共需要 numPlayers × 9 张牌

std::vector<std::vector<Card>> Deck::deal(int numPlayers) const {
    // 参数校验：2~6人，每人9张
    if (numPlayers < 2 || numPlayers > 6) {
        throw std::invalid_argument("Players must be 2~6");
    }

    int totalNeeded = numPlayers * 9;
    if (static_cast<int>(cards_.size()) < totalNeeded) {
        throw std::runtime_error("Not enough cards in deck");
    }

    // 准备结果：numPlayers 个空的手牌列表
    std::vector<std::vector<Card>> hands(numPlayers);
    for (auto& h : hands) {
        h.reserve(9);
    }

    // 逆时针发牌：从牌堆顶依次取牌，轮流发给每个人
    int cardIndex = 0;
    for (int round = 0; round < 9; round++) {           // 9轮
        for (int p = 0; p < numPlayers; p++) {           // 每轮每人1张
            hands[p].push_back(cards_[cardIndex]);
            cardIndex++;
        }
    }

    return hands;
}

// ====== 获取牌堆（调试用） ======

const std::vector<Card>& Deck::getCards() const {
    return cards_;
}

// ====== 获取牌堆张数 ======

int Deck::size() const {
    return static_cast<int>(cards_.size());
}
