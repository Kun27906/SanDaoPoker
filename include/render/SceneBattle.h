#pragma once
#include "render/SceneManager.h"
#include "render/CardSprite.h"
#include "ui/TextBox.h"
#include "ui/Avatar.h"
#include <array>

// 逐道展示比牌过程, 支持 2~6 人
class SceneBattle : public Scene {
public:
    explicit SceneBattle(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    void loadLine(int lineId, bool faceUp);
    void flipPlayer(int p);
    void revealWinner();
    void advance();

    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    TextBox info_;
    TextBox lineTag_;
    std::array<std::array<CardSprite, 3>, 6> cards_;
    std::array<Avatar, 6> seatAvatars_;
    int playerCount_ = 0;
    int showLine_ = 0;
    int phase_ = 0;
    int flipIndex_ = -1;
    float timer_ = 0.f;
    bool showNext_ = false;
};
