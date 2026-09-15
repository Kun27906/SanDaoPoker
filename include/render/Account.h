#pragma once
#include <string>

// "账号" = 一个本地存档,记录玩家持有的筹码余额。
// - 存档文件: <项目根>/game_data/save.dat
// - 初始资金: 500
// - 破产保护: 余额 < 100 时调用 topUp 自动补至 500
// - 可随时 reset 删除存档重新初始化

class Account {
public:
    static Account& instance();

    void load();               // 启动时调用:读存档;文件缺失/损坏则初始化为 500
    void save() const;         // 写回存档
    void reset();              // 删除存档并初始化回 500

    int balance() const { return balance_; }
    void setBalance(int v);    // 设置余额并立即保存
    int add(int delta);        // 余额增减,返回新余额并保存

    void setNickname(const std::string& n);   // 手动设置昵称并立即保存
    std::string ensureNickname();             // 无则随机生成并保存, 返回

    bool needsTopUp() const { return balance_ < 100; }  // 是否触发破产保护
    void topUp();              // 补充至 500 并保存

private:
    Account() = default;
    int balance_ = 500;
    std::string nickname_;     // 玩家昵称
    std::string path_ = "game_data/save.dat";  // 相对程序运行目录
};
