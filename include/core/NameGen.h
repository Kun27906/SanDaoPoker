#pragma once
#include <string>

// ====== NameGen 随机昵称生成(成员C) ======
// 由"前缀字 + 后缀字"组合出 2 字中文昵称(共 20x20=400 种)。
// makeUniqueNickname 在给定"已用名称表"上保证不重名(同场每人名称不同)。
// 用法:
//   std::string used[MAX_PLAYERS]; int n = 0;
//   used[n++] = makeNickname();
//   std::string nm = makeUniqueNickname(used, n);
std::string makeNickname();
std::string makeUniqueNickname(const std::string* used, int usedCount);
