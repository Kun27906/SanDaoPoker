#include "render/SceneResult.h"
#include "render/AssetManager.h"
#include "core/Room.h"
#include <cstdio>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;
}

SceneResult::SceneResult(SceneManager* mgr) : mgr_(mgr) {
    if (const sf::Texture* bg = AssetManager::instance().background()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

    // 结算本局(一次性)
    doSettle();

    // 标题
    char title[64];
    std::snprintf(title, sizeof(title), "第 %d 局结算", mgr_->room->currentRound);
    title_.setText(title);
    title_.setCharacterSize(34);
    title_.setColor(sf::Color(255, 215, 0));   // 金色(背景对比强烈)
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 40.f));

    // 本局结果文字(多行,左列,左上角定位)
    resultText_.setCharacterSize(20);
    resultText_.setColor(sf::Color(255, 190, 80));   // 亮橙(替代近白灰)
    resultText_.setPosition(sf::Vector2f(90.f, 110.f));

    // 总账排名(右列)
    rankText_.setCharacterSize(20);
    rankText_.setColor(sf::Color(255, 225, 140));
    rankText_.setPosition(sf::Vector2f(760.f, 110.f));

    // 按钮
    btnNext_.setText("下一局");
    btnNext_.setPosition(sf::Vector2f(300.f, 690.f));
    btnNext_.setSize(sf::Vector2f(300.f, 60.f));
    btnNext_.setCallback([this]() { nextRound(); });

    btnMenu_.setText("返回主菜单");
    btnMenu_.setPosition(sf::Vector2f(680.f, 690.f));
    btnMenu_.setSize(sf::Vector2f(300.f, 60.f));
    btnMenu_.setCallback([this]() { mgr_->changeTo(SceneId::Menu); });

    // 比赛打完:下一局禁用,显示最终排名
    if (finished_) {
        finalText_.setText("本场全部轮次已打完! 右侧为最终总账, 可返回主菜单重新选房");
        finalText_.setCharacterSize(20);
        finalText_.setColor(sf::Color(255, 120, 120));
        finalText_.setPosition(sf::Vector2f(300.f, 650.f));
    }
}

void SceneResult::doSettle() {
    if (settled_ || !mgr_->room) return;
    settled_ = true;

    // 结算本局(检查所有人交牌 -> 逐道比牌 -> 筹码变动,返回文字)
    std::string settleText = mgr_->room->settleRound();
    resultText_.setText(settleText);

    // 总账(按筹码排序)
    std::string rankText = mgr_->room->getRanking();
    rankText_.setText(rankText);

    // 比赛是否打完
    finished_ = mgr_->room->isFinished();
}

void SceneResult::nextRound() {
    if (finished_) return;  // 比赛已结束,不能开新局
    // 注意:不能在这里调 startNewRound —— SceneArrange 构造函数会统一开局,
    // 若这里也开一次,每局 currentRound 会 +2(出现 1->3->5 的奇数局 bug)
    mgr_->changeTo(SceneId::Arrange);
}

void SceneResult::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    btnMenu_.handleEvent(e, win);
    if (!finished_) btnNext_.handleEvent(e, win);
}

void SceneResult::update(float) {}

void SceneResult::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    resultText_.draw(win);
    rankText_.draw(win);
    if (finished_) finalText_.draw(win);
    btnNext_.draw(win);
    btnMenu_.draw(win);
}
