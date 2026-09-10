#include "render/SceneDeal.h"
#include "render/AssetManager.h"
#include "render/Account.h"
#include "render/SoundManager.h"
#include "core/NameGen.h"
#include "core/Room.h"
#include <cmath>
#include <cstdio>

namespace {
constexpr unsigned WW = 1280;
constexpr unsigned WH = 800;

// 手牌槽(与 SceneArrange 一致)
constexpr float HAND_X[9] = {90.f, 210.f, 330.f, 450.f, 570.f,
                             690.f, 810.f, 930.f, 1050.f};
constexpr float HAND_Y = 640.f;
constexpr float CARD_SCALE = 0.5f;         // 200x280 -> 100x140
constexpr float CARDW = 200.f * CARD_SCALE;
constexpr float CARDH = 280.f * CARD_SCALE;

// 牌堆
constexpr int   LAYERS = 54;               // 牌堆层数(素材)
constexpr float PILE_TEX_W = 689.f;
constexpr float PILE_TEX_H = 292.f;
constexpr float PILE_RIGHT = 985.f;        // 牌堆右端固定
constexpr float PILE_TOP = 56.f;
constexpr float EDGE = (PILE_TEX_W - 200.f) / (LAYERS - 1);   // 每层露边宽度

// 节奏
constexpr float DEAL_PER_CARD = 1.f / 3.f; // 每 3 张 1 秒
constexpr float FLIGHT_DUR = 0.22f;
constexpr float FADE_DUR = 0.5f;
constexpr float FLIP_DUR = 0.35f;

// 头像
constexpr float SELF_CX = 78.f, SELF_CY = 548.f, SELF_R = 30.f;
constexpr float AI_CX = 1188.f, AI_CY = 400.f, AI_R = 22.f, AI_GAP = 84.f;
}

SceneDeal::SceneDeal(SceneManager* mgr) : mgr_(mgr) {
    // 防御:房间不存在则建默认 4 人房
    if (!mgr_->room) {
        mgr_->room = std::make_unique<Room>();
        mgr_->room->setRoomConfig(7);
        std::string used[MAX_PLAYERS];
        int uc = 0;
        std::string meName = Account::instance().ensureNickname();
        used[uc++] = meName;
        mgr_->room->addPlayer(meName, false);
        for (int i = 1; i < mgr_->room->config.players; i++) {
            std::string nm = makeUniqueNickname(used, uc);
            used[uc++] = nm;
            mgr_->room->addPlayer(nm, true);
        }
    }

    // 背景
    if (const sf::Texture* bg = AssetManager::instance().background()) {
        bg_.setTexture(*bg);
        bg_.setScale(static_cast<float>(WW) / bg->getSize().x,
                     static_cast<float>(WH) / bg->getSize().y);
    }

    // 开局:洗牌 + 发牌 + 收底注(发牌动画只做表现, 真实牌已由 Room 发好)
    mgr_->room->startNewRound();

    // 本局牌背颜色(红/蓝/黑)与牌堆素材
    AssetManager::instance().rollBack();
    backIdx_ = AssetManager::instance().currentBack();
    if (const sf::Texture* pile = AssetManager::instance().deckPile(backIdx_)) {
        pile_.setTexture(*pile);
    }

    // 提示
    hint_.setText("发牌中...");
    hint_.setCharacterSize(26);
    hint_.setColor(sf::Color(255, 215, 0));
    hint_.centerOrigin();
    hint_.setPosition(sf::Vector2f(WW / 2.f, 30.f));

    chipBar_.setPosition(sf::Vector2f(WW - 250.f - 20.f, 16.f));

    // 头像: 本人左下, 他人右侧居中
    int pc = mgr_->room->playerCount;
    for (int i = 0; i < pc && i < MAX_PLAYERS; i++) {
        avatars_[i].setNickname(mgr_->room->players[i].name);
        avatars_[i].setTexture(AssetManager::instance().avatarTexture(i));   // 头像素材(占位色块)
        if (i == 0) {
            avatars_[i].setRadius(SELF_R);
            avatars_[i].setMinPlateWidth(150.f);   // 本人: 预留更长昵称空间
            avatars_[i].setSelfStyle(true);        // 名牌与圆心共线, 昵称居中
            avatars_[i].setCenter(sf::Vector2f(SELF_CX, SELF_CY));
        } else {
            avatars_[i].setRadius(AI_R);
            avatars_[i].setCenter(aiAvatarCenter(i));
        }
    }

    // 本人 9 张手牌(先记牌值, 发牌动画只是表现)
    for (int i = 0; i < 9; i++) {
        handCards_[i] = mgr_->room->players[0].hand[i];
        arrived_[i] = false;
    }

    total_ = pc * 9;
}

sf::Vector2f SceneDeal::aiAvatarCenter(int who) const {
    int n = mgr_->room->playerCount - 1;      // AI 数量
    if (n < 1) n = 1;
    float totalH = (n - 1) * AI_GAP;
    float y0 = AI_CY - totalH / 2.f;
    return sf::Vector2f(AI_CX, y0 + (who - 1) * AI_GAP);
}

bool SceneDeal::anyFlying() const {
    for (const Fly& f : flies_) {
        if (f.active) return true;
    }
    return false;
}

void SceneDeal::spawnNextCard() {
    Room* room = mgr_->room.get();
    int pc = room->playerCount;
    int round = dealt_ / pc;
    int who = dealt_ % pc;

    // 发牌前: 牌堆剩余宽度 → 前沿完整牌背(最左那张)的位置与大小(与素材等大)
    int remaining = LAYERS - dealt_;
    float w = (remaining <= 0) ? 0.f : (200.f + (remaining - 1) * EDGE);
    float frontCx = PILE_RIGHT - w + CARDW / 2.f;   // 前沿完整牌背中心
    float frontCy = PILE_TOP + CARDH / 2.f;

    Fly f;
    f.active = true;
    f.from = sf::Vector2f(frontCx, frontCy);
    f.fromS = 1.0f;                                 // 起点 = 堆叠素材中完整牌背等大
    f.t = 0.f;
    f.dur = FLIGHT_DUR;
    if (who == 0) {
        f.toPlayer = true;
        f.slot = round;
        f.to = sf::Vector2f(HAND_X[round] + CARDW / 2.f, HAND_Y + CARDH / 2.f);
        f.toS = CARD_SCALE;                         // 到手牌尺寸 0.5
    } else {
        f.toPlayer = false;
        f.who = who;
        f.to = aiAvatarCenter(who);
        f.toS = 0.18f;                              // 迅速变小飞入头像
    }
    flies_.push_back(f);

    dealt_++;
    SoundManager::instance().playDeal();    // 发牌音效
}

void SceneDeal::update(float dt) {
    // 飞行中的牌推进
    for (Fly& f : flies_) {
        if (!f.active) continue;
        f.t += dt / f.dur;
        if (f.t >= 1.f) {
            f.t = 1.f;
            if (f.toPlayer && f.slot >= 0 && f.slot < 9) arrived_[f.slot] = true;
            f.active = false;              // AI 牌到达后消失
        }
    }

    switch (phase_) {
        case Phase::Dealing: {
            dealTimer_ += dt;
            while (dealTimer_ >= DEAL_PER_CARD && dealt_ < total_) {
                dealTimer_ -= DEAL_PER_CARD;
                spawnNextCard();
            }
            if (dealt_ >= total_ && !anyFlying()) {
                int remainingLayers = LAYERS - dealt_;
                phase_ = (remainingLayers > 0) ? Phase::Fading : Phase::Flipping;
            }
            break;
        }
        case Phase::Fading: {
            fadeT_ += dt / FADE_DUR;
            if (fadeT_ >= 1.f) {
                fadeT_ = 1.f;
                phase_ = Phase::Flipping;
            }
            break;
        }
        case Phase::Flipping: {
            if (!flipSoundPlayed_) {
                flipSoundPlayed_ = true;
                SoundManager::instance().playFlip();   // 翻牌音效
                hint_.setText("翻开手牌...");
            }
            flipT_ += dt / FLIP_DUR;
            if (flipT_ >= 1.f) {
                mgr_->changeTo(SceneId::Arrange);      // 进入正式组牌
            }
            break;
        }
    }
}

void SceneDeal::drawCardAt(sf::RenderWindow& win, const Card& c, sf::Vector2f tl,
                           float sx, float sy, bool faceUp) const {
    const AssetManager& am = AssetManager::instance();
    const sf::Texture* t = faceUp ? am.cardTexture(c.getSuit(), c.getRank())
                                  : am.backTexture(backIdx_);
    if (!t) return;
    sf::Sprite sp(*t);
    // 牌面/牌背贴图统一 200x280, 直接按 sx,sy 缩放
    sp.setScale(sx, sy);
    sp.setPosition(tl);
    win.draw(sp);
}

void SceneDeal::draw(sf::RenderWindow& win) {
    if (bg_.getTexture()) win.draw(bg_);
    hint_.draw(win);

    // ---- 牌堆(从右边逐层切除一个白边并整体右移, 始终保留一张完整牌背) ----
    int remaining = LAYERS - dealt_;
    if (remaining > 0) {
        float w = 200.f + (remaining - 1) * EDGE;      // 剩余宽度(>=200, 留完整牌背)
        pile_.setTextureRect(sf::IntRect(0, 0, static_cast<int>(w),
                                         static_cast<int>(PILE_TEX_H)));
        pile_.setPosition(sf::Vector2f(PILE_RIGHT - w, PILE_TOP));   // 右端固定
        int alpha = 255;
        if (phase_ == Phase::Fading || phase_ == Phase::Flipping) {
            alpha = static_cast<int>(255.f * (1.f - fadeT_));
            if (alpha < 0) alpha = 0;
        }
        pile_.setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
        win.draw(pile_);
    }

    // ---- 本人手牌槽(发到的牌) ----
    for (int i = 0; i < 9; i++) {
        if (!arrived_[i]) continue;
        bool faceUp = (phase_ == Phase::Flipping && flipT_ >= 0.5f);
        float sxFactor = 1.f;
        if (phase_ == Phase::Flipping) {
            sxFactor = std::fabs(1.f - 2.f * flipT_);   // 水平翻牌
        }
        float w = CARDW * sxFactor;
        float x = HAND_X[i] + (CARDW - w) / 2.f;
        drawCardAt(win, handCards_[i], sf::Vector2f(x, HAND_Y),
                   CARD_SCALE * sxFactor, CARD_SCALE, faceUp);
    }

    // ---- 飞行中的牌 ----
    const sf::Texture* backTex = AssetManager::instance().backTexture(backIdx_);
    if (backTex) {
        for (const Fly& f : flies_) {
            if (!f.active) continue;
            float e = f.t * f.t * (3.f - 2.f * f.t);   // smoothstep
            sf::Vector2f p(f.from.x + (f.to.x - f.from.x) * e,
                           f.from.y + (f.to.y - f.from.y) * e);
            float s = f.fromS + (f.toS - f.fromS) * e;
            sf::Sprite sp(*backTex);
            sp.setScale(s, s);
            sp.setPosition(sf::Vector2f(p.x - 100.f * s, p.y - 140.f * s));  // 以中心对齐
            win.draw(sp);
        }
    }

    // ---- 头像(本人左下 / 他人右侧居中) ----
    for (int i = 0; i < mgr_->room->playerCount && i < MAX_PLAYERS; i++) {
        avatars_[i].draw(win);
    }

    chipBar_.draw(win, Account::instance().balance());
}

void SceneDeal::handleEvent(const sf::Event&, const sf::RenderWindow&) {
    // 发牌过程无交互
}
