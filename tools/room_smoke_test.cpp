// ====== Round/Room 完整对局冒烟验证 (成员D, 集成用) ======
// 覆盖: 16房间配置 -> 开新局(发牌+收底注) -> 全员随机交牌 -> 结算 -> 多轮 -> 总账
#include "core/Room.h"
#include "core/Round.h"
#include "core/RuleConfig.h"
#include "ai/AIPlayer.h"
#include <cstdio>
#include <random>

static std::mt19937 rng(12345);

// 随机交牌(模拟玩家/假AI, 不依赖界面)
static void randomArrange(Room& room, int seedOffset) {
    for (int p = 0; p < room.playerCount; p++) {
        int order[9];
        for (int i = 0; i < 9; i++) order[i] = i;
        // 前几个玩家用真 AI(胜率表在则加载), 其余随机
        static bool loaded = AIPlayer::loadWinRateTable("assets/ai/winrate.bin");
        if (p >= 1 && loaded) {
            AIPlayer::decideOrderStyled(room.players[p].hand, room.playerCount,
                                        AIPlayer::Difficulty::Greedy,
                                        AIPlayer::Style::Balanced, 0.3f, order);
        } else {
            std::shuffle(order, order + 9, rng);
        }
        room.players[p].arrangeByOrder(order);
        room.players[p].hasArranged = true;
    }
}

int main() {
    int pass = 0, fail = 0;
    for (int ci = 0; ci < ROOM_CONFIG_COUNT; ci++) {
        Room room;
        if (!room.setRoomConfig(ci)) { printf("[FAIL] 房间%d 配置失败\n", ci); fail++; continue; }
        const RoomConfig& cfg = ROOM_CONFIGS[ci];

        // 加满人
        bool ok = true;
        for (int i = 0; i < cfg.players; i++) {
            std::string name = "P" + std::to_string(i);
            if (!room.addPlayer(name, i > 0)) { ok = false; break; }
        }
        if (!ok || room.playerCount != cfg.players) {
            printf("[FAIL] 房间%d(%s) 加人失败 %d/%d\n", ci, cfg.name, room.playerCount, cfg.players);
            fail++; continue;
        }

        // 打满 rounds 局
        bool allOk = true;
        for (int r = 0; r < cfg.rounds; r++) {
            if (!room.startNewRound()) { allOk = false; break; }  // 发牌+收底注
            // 检查底注扣了(3份×ante)且进了池
            if (room.pools[0] + room.pools[1] + room.pools[2] != 3 * cfg.ante * cfg.players) allOk = false;
            // 全员交牌
            randomArrange(room, r);
            if (!Round::allArranged(room.players, room.playerCount)) { allOk = false; }
            // 结算
            std::string res = room.settleRound();
            if (res.empty()) allOk = false;
        }

        if (allOk && room.isFinished()) {
            // 总账能输出
            std::string rank = room.getRanking();
            if (rank.empty()) { printf("[FAIL] 房间%d(%s) 总账为空\n", ci, cfg.name); fail++; }
            else { printf("[PASS] 房间%d %-8s %d人 底注%d %d局 总账正常\n", ci, cfg.name, cfg.players, cfg.ante, cfg.rounds); pass++; }
        } else {
            printf("[FAIL] 房间%d(%s) 对局流程异常\n", ci, cfg.name);
            fail++;
        }
    }
    printf("===== 结果: %d 房间通过, %d 失败 =====\n", pass, fail);
    return fail == 0 ? 0 : 1;
}
