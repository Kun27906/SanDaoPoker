#include "render/SceneLobby.h"
#include "render/AssetManager.h"
#include "render/Account.h"
#include <array>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;
}

SceneLobby::SceneLobby(SceneManager* mgr) : mgr_(mgr) {
    if (const sf::Texture* bg = AssetManager::instance().menuBackground()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

    title_.setText("选择人数");
    title_.setCharacterSize(40);
    title_.setColor(sf::Color(255, 215, 0));
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 60.f));

    hint_.setText("点击人数进入对应房间列表");
    hint_.setCharacterSize(18);
    hint_.setColor(sf::Color(220, 220, 220));
    hint_.centerOrigin();
    hint_.setPosition(sf::Vector2f(WW / 2.f, 110.f));

    const char* names[5] = {"2 人", "3 人", "4 人", "5 人", "6 人"};
    for (int i = 0; i < 5; i++) {
        btnSeats_[i].setText(names[i]);
        btnSeats_[i].setPosition(sf::Vector2f(440.f, 170.f + i * 110.f));
        btnSeats_[i].setSize(sf::Vector2f(400.f, 82.f));
        btnSeats_[i].setCallback([this, i]() {
            mgr_->selectedPlayerCount = i + 2;   // 2..6
            mgr_->changeTo(SceneId::RoomSelect);
        });
    }

    chipBar_.setPosition(sf::Vector2f(WW - 250.f - 20.f, 16.f));

    // 重置账号(右下角; 两段确认: 第一次点击进入确认态, 再点一次执行)
    btnReset_.setText("重置账号");
    btnReset_.setPosition(sf::Vector2f(WW - 200.f, WH - 64.f));
    btnReset_.setSize(sf::Vector2f(180.f, 46.f));
    btnReset_.setCallback([this]() {
        if (!resetArmed_) {
            resetArmed_ = true;
            resetArmTimer_ = 0.f;
            btnReset_.setText("再点一次确认重置");
            btnReset_.setColors(sf::Color(200, 60, 50), sf::Color(240, 90, 70), sf::Color(150, 40, 30));
        } else {
            Account::instance().reset();   // 清空存档并初始化回 500
            resetArmed_ = false;
            btnReset_.setText("重置账号");
            btnReset_.setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
        }
    });

    // 破产补充弹窗(样式 + 进入大厅时检测余额<100)
    overlay_.setSize(sf::Vector2f(WW, WH));
    overlay_.setFillColor(sf::Color(0, 0, 0, 160));
    dialog_.setSize(sf::Vector2f(720.f, 240.f));
    dialog_.setPosition(sf::Vector2f((WW - 720.f) / 2.f, (WH - 240.f) / 2.f));
    dialog_.setFillColor(sf::Color(30, 40, 70));
    dialog_.setOutlineColor(sf::Color(255, 215, 0));
    dialog_.setOutlineThickness(3.f);
    // 文案两行, 整块居中于弹窗中央偏上(下方留给按钮)
    topUpText_.setText("您已破产！已为您注入初始资金至500筹码，\n祝您再接再厉！");
    topUpText_.setCharacterSize(24);
    topUpText_.setColor(sf::Color(255, 220, 130));
    topUpText_.centerOrigin();
    topUpText_.setPosition(sf::Vector2f(WW / 2.f, (WH - 240.f) / 2.f + 88.f));
    btnTopUpOk_.setText("确定");
    btnTopUpOk_.setPosition(sf::Vector2f(WW / 2.f - 90.f, (WH - 240.f) / 2.f + 165.f));
    btnTopUpOk_.setSize(sf::Vector2f(180.f, 50.f));
    btnTopUpOk_.setCallback([this]() {
        Account::instance().topUp();   // 补至 500
        pendingTopUp_ = false;
    });

    // 进入大厅: 检测余额是否 <100(破产边界), 不足则弹窗补充
    Account& acct = Account::instance();
    if (acct.needsTopUp()) {
        pendingTopUp_ = true;
        acct.topUp();   // 先补(弹窗仅作告知, 确定后关闭)
    }
}

void SceneLobby::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (pendingTopUp_) {
        btnTopUpOk_.handleEvent(e, win);   // 破产弹窗只响应确定
        return;
    }
    for (auto& b : btnSeats_) b.handleEvent(e, win);
    btnReset_.handleEvent(e, win);
}

void SceneLobby::update(float dt) {
    // 确认态 5 秒未二次点击则复原
    if (resetArmed_) {
        resetArmTimer_ += dt;
        if (resetArmTimer_ >= 5.f) {
            resetArmed_ = false;
            btnReset_.setText("重置账号");
            btnReset_.setColors(sf::Color(64, 120, 200), sf::Color(90, 160, 240), sf::Color(40, 85, 150));
        }
    }
}

void SceneLobby::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    hint_.draw(win);
    for (auto& b : btnSeats_) b.draw(win);
    btnReset_.draw(win);
    chipBar_.draw(win, Account::instance().balance());

    if (pendingTopUp_) {
        win.draw(overlay_);
        win.draw(dialog_);
        topUpText_.draw(win);
        btnTopUpOk_.draw(win);
    }
}
