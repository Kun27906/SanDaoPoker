#include "render/SceneArrange.h"
#include "render/AssetManager.h"
#include "core/Deck.h"

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;
}

SceneArrange::SceneArrange(SceneManager* mgr) : mgr_(mgr) {
    // 背景
    if (const sf::Texture* bg = AssetManager::instance().background()) {
        bg_.setTexture(*bg);
        float sx = static_cast<float>(WW) / bg->getSize().x;
        float sy = static_cast<float>(WH) / bg->getSize().y;
        bg_.setScale(sx, sy);
    }

    title_.setText("组牌界面(阶段3骨架)");
    title_.setCharacterSize(44);
    title_.setColor(sf::Color::White);
    title_.centerOrigin();
    title_.setPosition(sf::Vector2f(WW / 2.f, 70.f));

    hint_.setText("阶段2验证: 牌面贴图渲染 + 牌背 + 倒计时条");
    hint_.setCharacterSize(20);
    hint_.setColor(sf::Color(210, 210, 210));
    hint_.centerOrigin();
    hint_.setPosition(sf::Vector2f(WW / 2.f, 120.f));

    // 发 3 张随机牌展示(复用 A 成员 Deck)
    Deck deck;
    deck.shuffle();
    const std::vector<Card>& cards = deck.getCards();
    for (int i = 0; i < 3; i++) {
        cardSprites_[i].setCard(cards[i]);
        cardSprites_[i].setFaceUp(true);
        cardSprites_[i].setScale(0.9f);
        cardSprites_[i].setPosition(sf::Vector2f(360.f + i * 210.f, 220.f));
    }
    // 1 张牌背(蓝色)
    backSprite_.setCard(cards[3]);
    backSprite_.setFaceUp(false);
    backSprite_.setBackIndex(1);
    backSprite_.setScale(0.9f);
    backSprite_.setPosition(sf::Vector2f(990.f, 220.f));

    // 倒计时条(10秒,自动开始演示)
    countdown_ = CountdownBar(10.f, sf::Vector2f(390.f, 560.f), sf::Vector2f(500.f, 34.f));
    countdown_.start();

    btnBack_.setText("返回主菜单");
    btnBack_.setPosition(sf::Vector2f(440.f, 650.f));
    btnBack_.setSize(sf::Vector2f(400.f, 56.f));
    btnBack_.setCallback([this]() { mgr_->changeTo(SceneId::Menu); });
}

void SceneArrange::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    btnBack_.handleEvent(e, win);
}

void SceneArrange::update(float dt) {
    countdown_.update(dt);
}

void SceneArrange::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    title_.draw(win);
    hint_.draw(win);
    for (auto& cs : cardSprites_) cs.draw(win);
    backSprite_.draw(win);
    countdown_.draw(win);
    btnBack_.draw(win);
}
