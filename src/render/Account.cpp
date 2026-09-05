#include "render/Account.h"
#include <cstdio>
#include <fstream>
#include <sstream>

Account& Account::instance() {
    static Account inst;
    return inst;
}

void Account::load() {
    std::ifstream in(path_);
    if (in) {
        int v = 0;
        in >> v;
        if (in && v > 0) {
            balance_ = v;   // 读取成功
            return;
        }
    }
    // 不存在/损坏 -> 初始化为 500 并重建
    balance_ = 500;
    save();
}

void Account::save() const {
    // 确保 game_data 目录存在
    std::string dir = path_.substr(0, path_.find_last_of("/\\"));
    std::string mkdirCmd = "if not exist \"" + dir + "\" mkdir \"" + dir + "\"";
    std::system(mkdirCmd.c_str());

    std::ofstream out(path_);
    if (out) {
        out << balance_ << "\n";
    }
}

void Account::reset() {
    std::remove(path_.c_str());
    balance_ = 500;
    save();
}

void Account::setBalance(int v) {
    balance_ = v;
    save();
}

int Account::add(int delta) {
    balance_ += delta;
    save();
    return balance_;
}

void Account::topUp() {
    balance_ = 500;
    save();
}
