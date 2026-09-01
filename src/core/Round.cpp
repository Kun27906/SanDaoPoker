#include "Round.h"
#include <sstream>

// ====== 发牌 ======
// 洗牌之后，给每个玩家发 9 张牌

void Round::deal(Player* players, int playerCount, Deck& deck) {
    // 先洗牌（不传种子 = 真随机，每次都不一样）
    deck.shuffle();

    // 用发牌器拿每个人的 9 张牌
    std::vector<std::vector<Card>> hands = deck.deal(playerCount);

    // 把牌塞给每个玩家
    for (int p = 0; p < playerCount; p++) {
        players[p].clearRound();  // 清掉上一局的摆牌数据
        players[p].setHand(hands[p].data(), CARDS_PER_HAND);
    }
}

// ====== 收底注（三小池） ======
// 每人扣 3 份底注（头/中/尾道各 1 份），放进三个池子

void Round::collectAnte(Player* players, int playerCount, int ante, int pools[3]) {
    // 三个池子清零
    pools[0] = pools[1] = pools[2] = 0;

    for (int p = 0; p < playerCount; p++) {
        // 每人下 3 份底注
        players[p].chips -= ante * 3;
        // 分别放进三个池
        pools[0] += ante;
        pools[1] += ante;
        pools[2] += ante;
    }
}

// ====== 交牌锁定检查 ======
// 所有人都摆好牌（hasArranged = true）才能开始比牌

bool Round::allArranged(const Player* players, int playerCount) {
    for (int p = 0; p < playerCount; p++) {
        if (!players[p].hasArranged) {
            return false;  // 还有人不肯交牌
        }
    }
    return true;
}

// ====== 找出某一道的并列最大者 ======
// 例：4 人比头道，第 2、第 4 名都是豹子K → winners = {1, 3}，返回 2

int Round::findWinners(const Player* players, int playerCount, int lineId, int* winners) {
    HandResult best;      // 当前最大的牌型
    bool first = true;
    int count = 0;        // 并列人数

    for (int p = 0; p < playerCount; p++) {
        // 把这一道的 3 张牌装进数组
        const Card* line = players[p].getLine(lineId);
        std::vector<Card> three = { line[0], line[1], line[2] };
        HandResult r = HandEvaluator::evaluate(three);

        if (first) {
            best = r;
            count = 0;
            winners[count++] = p;
            first = false;
        } else {
            int cmp = HandEvaluator::compare(r, best);
            if (cmp > 0) {
                // 更大：清空重来
                best = r;
                count = 0;
                winners[count++] = p;
            } else if (cmp == 0) {
                // 一样大：并列，加进来
                winners[count++] = p;
            }
            // 更小：不理它
        }
    }
    return count;
}

// ====== 比较某一道（旧接口，保留给外部用） ======

int Round::compareLine(const Player* players, int playerCount, int lineId) {
    int winners[MAX_PLAYERS];
    int count = findWinners(players, playerCount, lineId, winners);
    if (count == 1) return winners[0];
    return -1;  // 平局
}

// ====== 比牌结算 ======
// 按 头道 → 中道 → 尾道 的顺序逐道比：
//   唯一赢家：拿走整个池子
//   并列最大：池子平分（除不尽的余数作废）
// 返回一段结果文字（每道谁赢、赢的什么牌型、筹码变化）

std::string Round::settle(Player* players, int playerCount, int pools[3]) {
    std::stringstream out;   // 拼结果文字
    const char* lineName[3] = { "头道", "中道", "尾道" };

    out << "===== 本局结果 =====" << std::endl;

    // 3 道逐一比较（轮次比牌）
    for (int line = 0; line < LINES_PER_HAND; line++) {
        int winners[MAX_PLAYERS];
        int count = findWinners(players, playerCount, line, winners);

        if (count == 1) {
            // ---- 唯一赢家：拿走整个池 ----
            int w = winners[0];
            players[w].chips += pools[line];   // 赢家收下池子
            players[w].score++;                // 统计：赢一道 +1

            // 打印赢家的牌型
            const Card* cards = players[w].getLine(line);
            std::vector<Card> three = { cards[0], cards[1], cards[2] };
            HandResult r = HandEvaluator::evaluate(three);

            out << lineName[line] << "：" << players[w].name
                << " 赢（" << r.name() << "），拿走 " << pools[line]
                << " 筹码" << std::endl;
        } else {
            // ---- 并列：平分赔付 ----
            int share = pools[line] / count;   // 每人分多少（整数除法，余数作废）
            for (int i = 0; i < count; i++) {
                players[winners[i]].chips += share;
                players[winners[i]].score++;
            }
            out << lineName[line] << "：打平！" << count << " 人平分 "
                << pools[line] << " 筹码（每人 " << share << "）" << std::endl;
        }
    }

    // 打印各玩家当前筹码
    out << "----- 当前筹码 -----" << std::endl;
    for (int p = 0; p < playerCount; p++) {
        out << players[p].name << "：" << players[p].chips << " 筹码" << std::endl;
    }
    return out.str();
}
