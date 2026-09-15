#include "core/Round.h"
#include <sstream>

// 洗牌并发给每人 9 张
void Round::deal(Player* players, int playerCount, Deck& deck) {
    deck.shuffle();

    std::vector<std::vector<Card>> hands = deck.deal(playerCount);

    for (int p = 0; p < playerCount; p++) {
        players[p].clearRound();
        players[p].setHand(hands[p].data(), CARDS_PER_HAND);
    }
}

// 收底注并均分三池
void Round::collectAnte(Player* players, int playerCount, int ante, int pools[3]) {
    pools[0] = pools[1] = pools[2] = 0;

    int total = 0;
    for (int p = 0; p < playerCount; p++) {
        players[p].chips -= ante;
        total += ante;
    }
    const int base = total / 3;
    pools[0] = base;
    pools[1] = base;
    pools[2] = base + (total % 3);
}

bool Round::allArranged(const Player* players, int playerCount) {
    for (int p = 0; p < playerCount; p++) {
        if (!players[p].hasArranged) {
            return false;
        }
    }
    return true;
}

// 返回并列赢家数, 下标写入 winners
int Round::findWinners(const Player* players, int playerCount, int lineId, int* winners) {
    HandResult best;
    bool first = true;
    int count = 0;

    for (int p = 0; p < playerCount; p++) {
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
                best = r;
                count = 0;
                winners[count++] = p;
            } else if (cmp == 0) {
                winners[count++] = p;
            }
        }
    }
    return count;
}

// 逐道比, 唯一赢家拿池, 并列平分
std::string Round::settle(Player* players, int playerCount, int pools[3]) {
    std::stringstream out;
    const char* lineName[3] = { "头道", "中道", "尾道" };

    out << "===== 本局结果 =====" << std::endl;

    for (int line = 0; line < LINES_PER_HAND; line++) {
        int winners[MAX_PLAYERS];
        int count = findWinners(players, playerCount, line, winners);

        if (count == 1) {
            int w = winners[0];
            players[w].chips += pools[line];
            players[w].score++;

            const Card* cards = players[w].getLine(line);
            std::vector<Card> three = { cards[0], cards[1], cards[2] };
            HandResult r = HandEvaluator::evaluate(three);

            out << lineName[line] << "：" << players[w].name
                << " 赢（" << r.name() << "），拿走 " << pools[line]
                << " 筹码" << std::endl;
        } else {
            int share = pools[line] / count;
            for (int i = 0; i < count; i++) {
                players[winners[i]].chips += share;
                players[winners[i]].score++;
            }
            out << lineName[line] << "：打平！" << count << " 人平分 "
                << pools[line] << " 筹码（每人 " << share << "）" << std::endl;
        }
    }

    out << "----- 当前筹码 -----" << std::endl;
    for (int p = 0; p < playerCount; p++) {
        out << players[p].name << "：" << players[p].chips << " 筹码" << std::endl;
    }
    return out.str();
}
