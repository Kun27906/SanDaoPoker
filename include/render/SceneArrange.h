#pragma once
#include "render/SceneManager.h"
#include "render/CardSprite.h"
#include "ui/Button.h"
#include "ui/TextBox.h"
#include "ui/CountdownBar.h"
#include "ui/ChipBar.h"
#include "ui/Avatar.h"
#include <array>

// 九张手牌分配到三道; 可拖拽吸附, 限时 40 秒
class SceneArrange : public Scene {
public:
    explicit SceneArrange(SceneManager* mgr);

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    void placeCard(int handIdx);
    void placeAt(int handIdx, int line, int pos);
    void returnToHand(int line, int pos);
    void resetArrange();
    void submit();
    void autoSubmit();
    bool allPlaced() const;
    void refreshSlotSprites();
    void rebuildLines();
    sf::Vector2f slotPos(int line, int pos) const;
    int  nearestSlot(const sf::Vector2f& cardTopLeft, float maxDist) const;   // line*3+pos, -1 无
    void beginDrag(int handIdx, int fromLine, int fromPos, const sf::Vector2f& mouse);
    void dropDrag();
    void startFlyBack(int handIdx);

    SceneManager* mgr_;
    sf::Sprite bg_;
    TextBox title_;
    std::array<CardSprite, 9> handSprites_;
    std::array<std::array<CardSprite, 3>, 3> lineSprites_;
    std::array<sf::RectangleShape, 9> handSlotRects_;
    std::array<std::array<sf::RectangleShape, 3>, 3> lineSlotRects_;
    std::array<Button, 3> lineBtns_;
    Button btnReset_;
    Button btnSubmit_;
    CountdownBar countdown_;
    std::array<bool, 9> handUsed_{};
    std::array<std::array<int, 3>, 3> slotHand_;
    int currentLine_ = 0;
    bool submitted_ = false;
    bool timeoutFired_ = false;
    ChipBar chipBar_;
    std::array<Avatar, MAX_PLAYERS> avatars_;

    CardSprite dragSprite_;
    bool pendingDrag_ = false;
    bool dragging_ = false;
    int  dragHand_ = -1;
    int  dragFromLine_ = -1;
    int  dragFromPos_ = -1;
    sf::Vector2f dragPos_{0.f, 0.f};
    sf::Vector2f dragGrab_{0.f, 0.f};
    sf::Vector2f pressPos_{0.f, 0.f};
    int  snapSlot_ = -1;

    CardSprite flySprite_;
    bool flying_ = false;
    int  flyHand_ = -1;
    sf::Vector2f flyFrom_{0.f, 0.f};
    sf::Vector2f flyTo_{0.f, 0.f};
    float flyT_ = 0.f;
};
