#pragma once
#include "render/SceneManager.h"
#include "ui/TextBox.h"
#include "ui/ChipBar.h"
#include "ui/Avatar.h"
#include "core/Card.h"
#include <array>
#include <vector>

// ====== 发牌动画场景(成员C, 阶段9) ======
// 进入即发牌(轮询: 本人 -> 各AI -> ... 每人 9 张):
//   · 开局先走"下注": bet 音效 + 筹码数字滚动扣减; 二者播完才开始发牌(不重叠)
//   · 牌堆用 cards/back/deck_pile_* (54 层右侧露边); 每发一张, 牌堆左端被取走(右端固定)
//   · 本人牌飞到底部手牌槽(牌背); AI 牌迅速缩小飞入右侧头像后消失
//   · 每 3 张牌耗时 1 秒; 用发牌音效
//   · 非 6 人时牌堆缩短到一定程度后淡化消失; 6 人则发完全部 54 张
//   · 发完后本人 9 张手牌统一快速翻转(翻牌音效) -> 进入组牌
class SceneDeal : public Scene {
public:
    explicit SceneDeal(SceneManager* mgr);
    void handleEvent(const sf::Event& e, const sf::RenderWindow& win) override;
    void update(float dt) override;
    void draw(sf::RenderWindow& win) override;

private:
    enum class Phase { Dealing, Fading, Flipping };
    struct Fly {
        bool active = false;
        bool toPlayer = false;
        int  slot = 0;                  // 本人: 手牌槽 0..8
        int  who = 0;                   // 他人: 玩家下标 1..
        sf::Vector2f from{0.f, 0.f}, to{0.f, 0.f};
        float t = 0.f, dur = 0.22f;
        float fromS = 0.5f, toS = 0.5f; // 均匀缩放
    };

    void spawnNextCard();
    bool anyFlying() const;
    sf::Vector2f aiAvatarCenter(int who) const;
    void drawCardAt(sf::RenderWindow& win, const Card& c, sf::Vector2f tl,
                    float sx, float sy, bool faceUp) const;

    SceneManager* mgr_;
    sf::Sprite bg_;
    sf::Sprite pile_;
    TextBox hint_;
    ChipBar chipBar_;
    std::array<Avatar, MAX_PLAYERS> avatars_;   // [0]=本人(左下), [1..]=他人(右中)
    std::array<Card, 9> handCards_{};
    std::array<bool, 9> arrived_{};
    std::vector<Fly> flies_;

    Phase phase_ = Phase::Dealing;
    int  dealt_ = 0;
    int  total_ = 0;
    float dealTimer_ = 0.f;
    float fadeT_ = 0.f;
    float flipT_ = 0.f;
    bool flipSoundPlayed_ = false;
    int  backIdx_ = 0;
    bool betDone_ = false;      // 下注音效+扣款动画是否播完(播完才开始发牌)
    float betWait_ = 0.f;
};
