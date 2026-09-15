#ifndef CARD_H
#define CARD_H

#include <string>
#include <cstdint>

// 花色：黑桃、红桃、梅花、方块
enum class Suit : uint8_t {
    Spade,    // 黑桃 ♠
    Heart,    // 红桃 ♥
    Club,     // 梅花 ♣
    Diamond   // 方块 ♦
};

// 点数：2~10、J、Q、K、A，加上小王和大王
enum class Rank : uint8_t {
    Two = 2, Three, Four, Five, Six, Seven, Eight, Nine, Ten,
    Jack, Queen, King, Ace,        // Ace = 14
    SmallJoker = 20,  // 小王
    BigJoker   = 21   // 大王
};

// 一张牌 = 花色 + 点数
// 大小王的花色无意义

class Card {
public:
    Card();                                  // 默认构造
    Card(Suit s, Rank r);                    // 普通牌
    Card(Rank jokerRank);                    // 大小王专用

    // 获取属性
    Suit getSuit() const;
    Rank getRank() const;

    // 判断是否为大小王
    bool isJoker() const;
    bool isBigJoker() const;
    bool isSmallJoker() const;

    // 获取点数的数值大小
    // 2=2, 3=3, ..., 10=10, J=11, Q=12, K=13, A=14
    // 小王=100, 大王=101
    int getRankValue() const;

    // 获取花色的整数值
    int getSuitValue() const;

    // 比较运算符
    bool operator<(const Card& other) const;
    bool operator>(const Card& other) const;
    bool operator==(const Card& other) const;
    bool operator!=(const Card& other) const;

private:
    Suit suit_;
    Rank rank_;
};

#endif // CARD_H
