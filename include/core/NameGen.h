#pragma once
#include <string>

// 昵称格式: 形容词 + 中心词 —— 60% 为 "xx的xx", 40% 省略"的"为 "xxxx"。
// 中心词: "前缀字 + 后缀字"组合
// 形容词: 同样用"前缀字 + 后缀字"组合
// 组合总数 400x400x2 = 320000 种, 足够同场 2~6 人不重名。
// makeUniqueNickname 在给定"已用名称表"上保证不重名。
// 用法:
// std::string used[MAX_PLAYERS]; int n = 0;
// used[n++] = makeNickname;
// std::string nm = makeUniqueNickname;
std::string makeNickname();
std::string makeUniqueNickname(const std::string* used, int usedCount);
