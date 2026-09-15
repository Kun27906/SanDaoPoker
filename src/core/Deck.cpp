#include "core/Deck.h"
#include <algorithm>
#include <random>
#include <stdexcept>

Deck::Deck() {
    reset();
}

void Deck::reset() {
    cards_.clear();
    cards_.reserve(54);

    for (int s = 0; s < 4; s++) {
        for (int r = 2; r <= 14; r++) {
            cards_.emplace_back(
                static_cast<Suit>(s),
                static_cast<Rank>(r)
            );
        }
    }

    cards_.emplace_back(Rank::SmallJoker);
    cards_.emplace_back(Rank::BigJoker);
}

// seed 为 0 时取随机种子, 否则用固定种子
void Deck::shuffle(unsigned seed) {
    std::mt19937 rng;

    if (seed == 0) {
        std::random_device rd;
        rng.seed(rd());
    } else {
        rng.seed(seed);
    }

    std::shuffle(cards_.begin(), cards_.end(), rng);
}

// 每人 9 张, 逐轮轮流发
std::vector<std::vector<Card>> Deck::deal(int numPlayers) const {
    if (numPlayers < 2 || numPlayers > 6) {
        throw std::invalid_argument("Players must be 2~6");
    }

    int totalNeeded = numPlayers * 9;
    if (static_cast<int>(cards_.size()) < totalNeeded) {
        throw std::runtime_error("Not enough cards in deck");
    }

    std::vector<std::vector<Card>> hands(numPlayers);
    for (auto& h : hands) {
        h.reserve(9);
    }

    int cardIndex = 0;
    for (int round = 0; round < 9; round++) {
        for (int p = 0; p < numPlayers; p++) {
            hands[p].push_back(cards_[cardIndex]);
            cardIndex++;
        }
    }

    return hands;
}

int Deck::size() const {
    return static_cast<int>(cards_.size());
}
