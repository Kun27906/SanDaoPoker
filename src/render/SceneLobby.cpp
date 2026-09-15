#include "render/SceneLobby.h"
#include "render/AssetManager.h"
#include "render/Account.h"
#include "ui/PanelFrame.h"
#include <array>

namespace {
constexpr unsigned WW = layout::WINDOW_W;
constexpr unsigned WH = layout::WINDOW_H;
}

SceneLobby::SceneLobby(SceneManager* mgr) : mgr_(mgr) {
    scene_setup::background(bg_, AssetManager::instance().menuBackground(), WW, WH);

    title_.setText("选择人数");
    title_.setCharacterSize(40);
    title_.setColor(sf::Color(255, 215, 0));
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 60.f));

    const char* names[5] = {"2 人", "3 人", "4 人", "5 人", "6 人"};
    for (int i = 0; i < 5; i++) {
        btnSeats_[i].setText(names[i]);
        btnSeats_[i].setPosition(sf::Vector2f(440.f, 170.f + i * 110.f));
        btnSeats_[i].setSize(sf::Vector2f(400.f, 82.f));
        btnSeats_[i].setCallback([this, i]() {
            mgr_->selectedPlayerCount = i + 2;
            mgr_->changeTo(SceneId::RoomSelect);
        });
    }

    scene_setup::chipBar(chipBar_, WW);
    scene_setup::selfAvatar(selfAvatar_, WW);
    scene_setup::versionBadge(versionBadge_, WH);

    profile_.bind(&selfAvatar_);

    btnReset_.setText("重置账号");
    btnReset_.setPosition(sf::Vector2f(WW - 200.f, WH - 64.f));
    btnReset_.setSize(sf::Vector2f(180.f, 46.f));
    btnReset_.setCallback([this]() {
        if (!resetArmed_) {
            resetArmed_ = true;
            resetArmTimer_ = 0.f;
            btnReset_.setText("再点一次确认重置");
            btnReset_.setTint(sf::Color(255, 140, 140));
        } else {
            Account::instance().reset();
            selfAvatar_.setNickname(Account::instance().ensureNickname());
            resetArmed_ = false;
            btnReset_.setText("重置账号");
            btnReset_.setTint(sf::Color::White);
        }
    });

    overlay_.setSize(sf::Vector2f(WW, WH));
    overlay_.setFillColor(sf::Color(0, 0, 0, 160));
    dialog_.setSize(sf::Vector2f(720.f, 240.f));
    dialog_.setPosition(sf::Vector2f((WW - 720.f) / 2.f, (WH - 240.f) / 2.f));
    dialog_.setFillColor(sf::Color(30, 40, 70));
    dialog_.setOutlineColor(sf::Color(255, 215, 0));
    dialog_.setOutlineThickness(3.f);
    topUpText_.setText("您已破产！已为您注入初始资金至500筹码，\n祝您再接再厉！");
    topUpText_.setCharacterSize(24);
    topUpText_.setColor(sf::Color(255, 220, 130));
    topUpText_.centerOrigin();
    topUpText_.setPosition(sf::Vector2f(WW / 2.f, (WH - 240.f) / 2.f + 88.f));
    btnTopUpOk_.setText("确定");
    btnTopUpOk_.setPosition(sf::Vector2f(WW / 2.f - 90.f, (WH - 240.f) / 2.f + 165.f));
    btnTopUpOk_.setSize(sf::Vector2f(180.f, 50.f));
    btnTopUpOk_.setCallback([this]() {
        Account::instance().topUp();
        pendingTopUp_ = false;
    });

    // 破产则先补足再弹窗
    Account& acct = Account::instance();
    if (acct.needsTopUp()) {
        pendingTopUp_ = true;
        acct.topUp();
    }
}

void SceneLobby::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (pendingTopUp_) {
        btnTopUpOk_.handleEvent(e, win);
        return;
    }
    if (versionBadge_.handleEvent(e, win)) return;
    if (chipBar_.handleEvent(e, win)) return;
    if (profile_.handleEvent(e, win)) return;
    for (auto& b : btnSeats_) b.handleEvent(e, win);
    btnReset_.handleEvent(e, win);
}

void SceneLobby::onHomePressed() {
    mgr_->changeTo(SceneId::Title);
}

void SceneLobby::update(float dt) {
    profile_.update(dt);
    // 确认态 5 秒未二次点击则复原
    if (resetArmed_) {
        resetArmTimer_ += dt;
        if (resetArmTimer_ >= 5.f) {
            resetArmed_ = false;
            btnReset_.setText("重置账号");
            btnReset_.setTint(sf::Color::White);
        }
    }
}

void SceneLobby::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    for (auto& b : btnSeats_) b.draw(win);
    btnReset_.draw(win);
    chipBar_.setImmediate(Account::instance().balance());
    chipBar_.draw(win);
    selfAvatar_.draw(win);
    versionBadge_.draw(win);

    if (pendingTopUp_) {
        win.draw(overlay_);
        win.draw(dialog_);
        panel_frame::draw(win, sf::FloatRect(dialog_.getPosition(), dialog_.getSize()));
        topUpText_.draw(win);
        btnTopUpOk_.draw(win);
    }

    profile_.draw(win);
}
