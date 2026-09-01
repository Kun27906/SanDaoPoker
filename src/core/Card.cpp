#include "core/Card.h"
#include <stdexcept>

// ====== 构造函数 ======

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

// ====== 获取属性 ======

Suit Card::getSuit() const {
    return suit_;
}

Rank Card::getRank() const {
    return rank_;
}

// ====== 判断是否为大小王 ======

bool Card::isJoker() const {
    return rank_ == Rank::SmallJoker || rank_ == Rank::BigJoker;
}

bool Card::isBigJoker() const {
    return rank_ == Rank::BigJoker;
}

bool Card::isSmallJoker() const {
    return rank_ == Rank::SmallJoker;
}

// ====== 获取点数的数值大小 ======
// 2=2, 3=3, ..., 10=10, J=11, Q=12, K=13, A=14
// 小王=100, 大王=101（确保大于所有普通牌）

int Card::getRankValue() const {
    if (rank_ == Rank::BigJoker)   return 101;
    if (rank_ == Rank::SmallJoker) return 100;
    return static_cast<int>(rank_);  // 2~14
}

// ====== 获取花色的整数值 ======

int Card::getSuitValue() const {
    return static_cast<int>(suit_);
}

// ====== 转为可读字符串 ======

std::string Card::toString() const {
    // 大小王
    if (rank_ == Rank::BigJoker)   return "BigJoker";
    if (rank_ == Rank::SmallJoker) return "SmallJoker";

    // 花色符号
    const char* suitStr[] = { "S", "H", "C", "D" };
    int s = static_cast<int>(suit_);
    std::string result = suitStr[s];

    // 点数文字
    switch (rank_) {
        case Rank::Two:   result += "2";   break;
        case Rank::Three: result += "3";   break;
        case Rank::Four:  result += "4";   break;
        case Rank::Five:  result += "5";   break;
        case Rank::Six:   result += "6";   break;
        case Rank::Seven: result += "7";   break;
        case Rank::Eight: result += "8";   break;
        case Rank::Nine:  result += "9";   break;
        case Rank::Ten:   result += "10";  break;
        case Rank::Jack:  result += "J";   break;
        case Rank::Queen: result += "Q";   break;
        case Rank::King:  result += "K";   break;
        case Rank::Ace:   result += "A";   break;
        default: break;
    }
    return result;
}

// ====== 比较运算符 ======

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
