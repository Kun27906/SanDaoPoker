#include "render/Account.h"
#include "core/NameGen.h"
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
        std::string line1;
        if (std::getline(in, line1)) {
            std::istringstream ss(line1);
            int v = 0;
            ss >> v;
            if (ss && v > 0) {
                balance_ = v;                 // 第 1 行: 余额
                std::string nm;
                if (std::getline(in, nm)) {   // 第 2 行: 昵称(旧存档可能缺失)
                    while (!nm.empty() &&
                           (nm.back() == '\r' || nm.back() == '\n' || nm.back() == ' ')) {
                        nm.pop_back();
                    }
                    nickname_ = nm;
                }
                return;
            }
        }
    }
    // 不存在/损坏 -> 初始化为 500 并重建(昵称留空, 首次使用时生成)
    balance_ = 500;
    nickname_.clear();
    save();
}

void Account::save() const {
    // 确保 game_data 目录存在
    std::string dir = path_.substr(0, path_.find_last_of("/\\"));
    std::string mkdirCmd = "if not exist \"" + dir + "\" mkdir \"" + dir + "\"";
    std::system(mkdirCmd.c_str());

    std::ofstream out(path_);
    if (out) {
        out << balance_ << "\n" << nickname_ << "\n";   // 余额 + 昵称
    }
}

void Account::reset() {
    std::remove(path_.c_str());
    balance_ = 500;
    nickname_.clear();   // 重置账号同时重置昵称(下次生成新的)
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

void Account::setNickname(const std::string& n) {
    nickname_ = n;
    save();
}

std::string Account::ensureNickname() {
    if (nickname_.empty()) {
        nickname_ = makeNickname();
        save();
    }
    return nickname_;
}

void Account::topUp() {
    balance_ = 500;
    save();
}
