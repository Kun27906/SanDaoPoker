// ====== 胜率表生成器 (成员B 训练工具) ======
// 用法: gen_winrate_table [输出路径]
// 默认输出: assets/ai/winrate.bin
#include "ai/AIPlayer.h"
#include <cstdio>
#include <string>

int main(int argc, char** argv) {
    std::string path = "assets/ai/winrate.bin";
    if (argc > 1) path = argv[1];

    std::printf("===== SanDaoPoker AI 胜率表生成 =====\n");
    std::printf("枚举全部 C(54,3)=24804 种三张牌组合, 逐个 vs 随机对手(20825种)精确计算胜率\n");
    std::printf("等价于让 AI 离线'学习'所有手牌的强弱, 之后查表 O(1) 决策\n\n");

    bool ok = AIPlayer::generateWinRateTable(path, true);
    if (!ok) {
        std::printf("失败: 无法写入 %s\n", path.c_str());
        return 1;
    }
    std::printf("\n胜率表生成完毕!\n");
    return 0;
}