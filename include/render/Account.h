#pragma once
#include <string>

// ====== Account 玩家账号(本地存档,成员C) ======
// "账号" = 一个本地存档(唯一),记录玩家持有的筹码余额。
//   - 存档文件: <项目根>/game_data/save.dat (一行整数)
//   - 初始资金: 500
//   - 破产保护: 余额 < 100 时调用 topUp() 自动补至 500(由 UI 弹窗告知用户)
//   - 可随时 reset() 删除存档重新初始化(再进入游戏时自动重建)

class Account {
public:
    static Account& instance();

    void load();               // 启动时调用:读存档;文件缺失/损坏则初始化为 500
    void save() const;         // 写回存档
    void reset();              // 删除存档并初始化回 500(重新自动创建)

    int balance() const { return balance_; }
    void setBalance(int v);    // 设置余额并立即保存
    int add(int delta);        // 余额增减(delta 可为负),返回新余额并保存

    bool needsTopUp() const { return balance_ < 100; }  // 是否触发破产保护
    void topUp();              // 补充至 500 并保存

private:
    Account() = default;
    int balance_ = 500;
    std::string path_ = "game_data/save.dat";  // 相对程序运行目录(项目根)
};
