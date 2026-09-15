#pragma once
#include <string>

// ====== NameGen 随机昵称生成(成员C) ======
// 昵称格式: "xx的xx" —— 形容词 + "的" + 二字中心词。
//   中心词: "前缀字 + 后缀字"组合(20x20=400 种, 如"孤影" "疾刃")
//   形容词: 同样用"前缀字 + 后缀字"组合(20x20=400 种, 如"孤傲" "冷冽")
// 合计 400x400 = 160000 种, 足够同场 2~6 人不重名。
// makeUniqueNickname 在给定"已用名称表"上保证不重名(同场每人名称不同)。
// 用法:
//   std::string used[MAX_PLAYERS]; int n = 0;
//   used[n++] = makeNickname();
//   std::string nm = makeUniqueNickname(used, n);
std::string makeNickname();
std::string makeUniqueNickname(const std::string* used, int usedCount);
