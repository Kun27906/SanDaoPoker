#pragma once
#include <string>

// 本地存档账号: 筹码余额与昵称; 初始 500, 余额低于 100 触发破产保护
class Account {
public:
    static Account& instance();

    void load();
    void save() const;
    void reset();

    int balance() const { return balance_; }
    void setBalance(int v);
    int add(int delta);

    void setNickname(const std::string& n);
    std::string ensureNickname();

    bool needsTopUp() const { return balance_ < 100; }
    void topUp();

private:
    Account() = default;
    int balance_ = 500;
    std::string nickname_;
    std::string path_ = "game_data/save.dat";
};
