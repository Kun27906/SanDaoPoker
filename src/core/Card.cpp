#include "core/Card.h"
#include <stdexcept>

Card::Card() : suit_(Suit::Spade), rank_(Rank::SmallJoker) {}

Card::Card(Suit s, Rank r) : suit_(s), rank_(r) {
    // 普通牌构造：r 必须是 2~A
    if (r == Rank::SmallJoker || r == Rank::BigJoker) {
        throw std::invalid_argument("Use Card(Rank) for jokers");
    }
}

Card::Card(Rank jokerRank) : suit_(Suit::Spade), rank_(jokerRank) {
    // 大小王专用构造：jokerRank 必须是 SmallJoker 或 BigJoker
    if (jokerRank != Rank::SmallJoker && jokerRank != Rank::BigJoker) {
        throw std::invalid_argument("Card(Rank) is for jokers only");
    }
}

Suit Card::getSuit() const {
    return suit_;
}

Rank Card::getRank() const {
    return rank_;
}

bool Card::isJoker() const {
    return rank_ == Rank::SmallJoker || rank_ == Rank::BigJoker;
}

bool Card::isBigJoker() const {
    return rank_ == Rank::BigJoker;
}

bool Card::isSmallJoker() const {
    return rank_ == Rank::SmallJoker;
}

// 2=2, 3=3, ..., 10=10, J=11, Q=12, K=13, A=14
// 小王=100, 大王=101

int Card::getRankValue() const {
    if (rank_ == Rank::BigJoker)   return 101;
    if (rank_ == Rank::SmallJoker) return 100;
    return static_cast<int>(rank_);  // 2~14
}

int Card::getSuitValue() const {
    return static_cast<int>(suit_);
}

bool Card::operator<(const Card& other) const {
    return getRankValue() < other.getRankValue();
}

bool Card::operator>(const Card& other) const {
    return getRankValue() > other.getRankValue();
}

bool Card::operator==(const Card& other) const {
    return rank_ == other.rank_ && suit_ == other.suit_;
}

bool Card::operator!=(const Card& other) const {
    return !(*this == other);
}
