#ifndef CARD_H
#define CARD_H

#include <string>
#include <cstdint>

// 黑桃 红桃 梅花 方块
enum class Suit : uint8_t {
    Spade,
    Heart,
    Club,
    Diamond
};

// Ace = 14; 小王 = 20, 大王 = 21
enum class Rank : uint8_t {
    Two = 2, Three, Four, Five, Six, Seven, Eight, Nine, Ten,
    Jack, Queen, King, Ace,
    SmallJoker = 20,
    BigJoker   = 21
};

// 一张牌 = 花色 + 点数; 大小王花色无意义
class Card {
public:
    Card();
    Card(Suit s, Rank r);
    Card(Rank jokerRank);

    Suit getSuit() const;
    Rank getRank() const;

    bool isJoker() const;
    bool isBigJoker() const;
    bool isSmallJoker() const;

    // 2..10 取原值, J=11 Q=12 K=13 A=14, 小王=100 大王=101
    int getRankValue() const;
    int getSuitValue() const;

    bool operator<(const Card& other) const;
    bool operator>(const Card& other) const;
    bool operator==(const Card& other) const;
    bool operator!=(const Card& other) const;

private:
    Suit suit_;
    Rank rank_;
};

#endif
