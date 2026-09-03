#include "render/AssetManager.h"
#include <cstdio>

AssetManager& AssetManager::instance() {
    static AssetManager inst;
    return inst;
}

bool AssetManager::loadAll() {
    if (loaded_) return true;
    loaded_ = loadCardTextures();
    loadMiscTextures();
    return loaded_;
}

// 花色枚举 -> 目录名(复数)
static const char* suitDir(Suit s) {
    switch (s) {
        case Suit::Spade:   return "spades";
        case Suit::Heart:   return "hearts";
        case Suit::Club:    return "clubs";
        case Suit::Diamond: return "diamonds";
    }
    return "spades";
}

// Rank -> 文件名(A/2..10/J/Q/K,见 assets/README.md)
static const char* rankName(Rank r) {
    switch (r) {
        case Rank::Ace:   return "A";
        case Rank::Jack:  return "J";
        case Rank::Queen: return "Q";
        case Rank::King:  return "K";
        default: {
            int v = static_cast<int>(r);
            if (v >= 2 && v <= 10) {
                static char buf[4];
                std::snprintf(buf, sizeof(buf), "%d", v);
                return buf;
            }
        }
    }
    return nullptr;  // 大小王
}

// Rank -> 贴图数组下标 0..12 (2=0 ... A=12); 大小王返回 -1
static int cardIndex(Rank r) {
    int v = static_cast<int>(r);
    if (v >= 2 && v <= 10) return v - 2;  // 2..10 -> 0..8
    if (v == 11) return 9;                // J
    if (v == 12) return 10;               // Q
    if (v == 13) return 11;               // K
    if (v == 14) return 12;               // A
    return -1;                            // 大小王
}

bool AssetManager::loadCardTextures() {
    bool allOk = true;
    const Suit suits[4] = {Suit::Spade, Suit::Heart, Suit::Club, Suit::Diamond};
    const Rank ranks[13] = {
        Rank::Two, Rank::Three, Rank::Four, Rank::Five, Rank::Six, Rank::Seven,
        Rank::Eight, Rank::Nine, Rank::Ten, Rank::Jack, Rank::Queen, Rank::King,
        Rank::Ace
    };
    for (int s = 0; s < 4; s++) {
        for (int i = 0; i < 13; i++) {
            const char* rn = rankName(ranks[i]);
            if (!rn) continue;
            char path[128];
            std::snprintf(path, sizeof(path),
                          "assets/cards/%s/%s.png", suitDir(suits[s]), rn);
            if (!cardTex_[s][i].loadFromFile(path)) {
                allOk = false;
                std::fprintf(stderr, "[AssetManager] 加载失败: %s\n", path);
            }
        }
    }
    // 大小王(Jokers 文件夹,由 tools/gen_jokers.py 生成)
    if (!jokerTex_[0].loadFromFile("assets/cards/Jokers/small.png")) {
        allOk = false;
        std::fprintf(stderr, "[AssetManager] 加载失败: assets/cards/Jokers/small.png\n");
    }
    if (!jokerTex_[1].loadFromFile("assets/cards/Jokers/big.png")) {
        allOk = false;
        std::fprintf(stderr, "[AssetManager] 加载失败: assets/cards/Jokers/big.png\n");
    }
    return allOk;
}

bool AssetManager::loadMiscTextures() {
    const char* backs[3] = {"red", "blue", "black"};
    for (int i = 0; i < 3; i++) {
        char path[128];
        std::snprintf(path, sizeof(path), "assets/cards/back/%s.png", backs[i]);
        sf::Texture t;
        if (t.loadFromFile(path)) {
            backTex_.push_back(t);
        } else {
            std::fprintf(stderr, "[AssetManager] 加载失败: %s\n", path);
        }
    }
    if (!bgTex_.loadFromFile("assets/ui/backgrounds/table_bg.png")) {
        std::fprintf(stderr, "[AssetManager] 加载失败: assets/ui/backgrounds/table_bg.png\n");
    }
    // 主菜单背景(单独一张;失败则场景代码回退到 table_bg)
    if (!menuTex_.loadFromFile("assets/ui/backgrounds/menu.jpg")) {
        std::fprintf(stderr, "[AssetManager] 加载失败: assets/ui/backgrounds/menu.jpg\n");
    }
    const char* btnStates[4] = {"normal", "hover", "pressed", "disabled"};
    for (int i = 0; i < 4; i++) {
        char path[128];
        std::snprintf(path, sizeof(path), "assets/ui/buttons/btn_%s.png", btnStates[i]);
        if (!btnTex_[i].loadFromFile(path)) {
            std::fprintf(stderr, "[AssetManager] 加载失败: %s\n", path);
        }
    }
    return true;
}

const sf::Texture* AssetManager::cardTexture(Suit s, Rank r) const {
    // 大小王:Jokers 贴图 [0]=small [1]=big
    if (r == Rank::SmallJoker) {
        return jokerTex_[0].getSize().x > 0 ? &jokerTex_[0] : nullptr;
    }
    if (r == Rank::BigJoker) {
        return jokerTex_[1].getSize().x > 0 ? &jokerTex_[1] : nullptr;
    }
    int idx = cardIndex(r);
    if (idx < 0 || idx > 12) return nullptr;
    int si = static_cast<int>(s);
    if (si < 0 || si > 3) return nullptr;
    const sf::Texture& t = cardTex_[si][idx];
    return t.getSize().x > 0 ? &t : nullptr;
}

const sf::Texture* AssetManager::backTexture(int index) const {
    if (backTex_.empty()) return nullptr;
    int i = (index < 0 || index >= static_cast<int>(backTex_.size())) ? 0 : index;
    return &backTex_[i];
}

const sf::Texture* AssetManager::background() const {
    return bgTex_.getSize().x > 0 ? &bgTex_ : nullptr;
}

const sf::Texture* AssetManager::menuBackground() const {
    if (menuTex_.getSize().x > 0) return &menuTex_;
    return bgTex_.getSize().x > 0 ? &bgTex_ : nullptr;  // 回退到桌面背景
}

const sf::Texture* AssetManager::buttonTexture(int state) const {
    if (state < 0 || state > 3) return nullptr;
    return btnTex_[state].getSize().x > 0 ? &btnTex_[state] : nullptr;
}
