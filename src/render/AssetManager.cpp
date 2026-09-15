#include "render/AssetManager.h"
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cmath>
#include <filesystem>
#include <random>
#include <algorithm>

AssetManager& AssetManager::instance() {
    static AssetManager inst;
    return inst;
}

bool AssetManager::loadAll() {
    if (loaded_) return true;
    loaded_ = loadCardTextures();
    loadMiscTextures();
    reloadCustomAvatar();
    return loaded_;
}

static const char* suitDir(Suit s) {
    switch (s) {
        case Suit::Spade:   return "spades";
        case Suit::Heart:   return "hearts";
        case Suit::Club:    return "clubs";
        case Suit::Diamond: return "diamonds";
    }
    return "spades";
}

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
    return nullptr;
}

// Rank 到贴图下标 0..12, 大小王返回 -1
static int cardIndex(Rank r) {
    int v = static_cast<int>(r);
    if (v >= 2 && v <= 10) return v - 2;
    if (v == 11) return 9;
    if (v == 12) return 10;
    if (v == 13) return 11;
    if (v == 14) return 12;
    return -1;
}

// 圆形裁剪: 圆外 alpha 置 0, 边缘 1 像素羽化
static void cropToCircle(sf::Image& img) {
    const unsigned w = img.getSize().x, h = img.getSize().y;
    if (w == 0 || h == 0) return;
    const float ccx = (w - 1) * 0.5f, ccy = (h - 1) * 0.5f;
    const float rad = std::min(w, h) * 0.5f;
    for (unsigned y = 0; y < h; y++) {
        for (unsigned x = 0; x < w; x++) {
            const float dx = static_cast<float>(x) - ccx;
            const float dy = static_cast<float>(y) - ccy;
            const float d = std::sqrt(dx * dx + dy * dy);
            const float inside = rad - d;
            if (inside <= 0.f) {
                sf::Color col = img.getPixel(x, y);
                col.a = 0;
                img.setPixel(x, y, col);
            } else if (inside < 1.f) {
                sf::Color col = img.getPixel(x, y);
                col.a = static_cast<sf::Uint8>(col.a * inside);
                img.setPixel(x, y, col);
            }
        }
    }
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
    if (!bgTex_.loadFromFile("assets/ui/backgrounds/table_bg.jpg")) {
        std::fprintf(stderr, "[AssetManager] 加载失败: assets/ui/backgrounds/table_bg.jpg\n");
    }
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
    // 筹码按区间命名: chip_<min>-<max>.png, 空侧表示无界
    {
        chipDefs_.clear();
        namespace fs = std::filesystem;
        std::error_code ec;
        if (fs::exists("assets/ui/chips", ec)) {
            for (auto& entry : fs::directory_iterator("assets/ui/chips", ec)) {
                if (!entry.is_regular_file(ec)) continue;
                if (entry.path().extension() != ".png") continue;
                std::string stem = entry.path().stem().string();
                if (stem.rfind("chip_", 0) != 0) continue;
                std::string range = stem.substr(5);
                int lo = INT_MIN, hi = INT_MAX;
                std::size_t dash = range.find('-');
                if (dash == std::string::npos) {
                    lo = hi = std::atoi(range.c_str());
                } else {
                    std::string a = range.substr(0, dash);
                    std::string b = range.substr(dash + 1);
                    if (!a.empty()) lo = std::atoi(a.c_str());
                    if (!b.empty()) hi = std::atoi(b.c_str());
                }
                ChipDef def;
                def.lo = lo;
                def.hi = hi;
                if (def.tex.loadFromFile(entry.path().string())) {
                    chipDefs_.push_back(def);
                } else {
                    std::fprintf(stderr, "[AssetManager] 加载失败: %s\n",
                                 entry.path().string().c_str());
                }
            }
        }
    }
    {
        icons_.clear();
        namespace fs = std::filesystem;
        std::error_code ec;
        if (fs::exists("assets/ui/icons", ec)) {
            for (auto& entry : fs::directory_iterator("assets/ui/icons", ec)) {
                if (!entry.is_regular_file(ec)) continue;
                if (entry.path().extension() != ".png") continue;
                std::string stem = entry.path().stem().string();
                sf::Texture t;
                if (t.loadFromFile(entry.path().string())) {
                    icons_[stem] = std::move(t);
                } else {
                    std::fprintf(stderr, "[AssetManager] 加载失败: %s\n",
                                 entry.path().string().c_str());
                }
            }
        }
    }
    {
        const char* piles[3] = {"red", "blue", "black"};
        for (int i = 0; i < 3; i++) {
            char path[128];
            std::snprintf(path, sizeof(path), "assets/cards/back/deck_pile_%s.png", piles[i]);
            if (!pileTex_[i].loadFromFile(path)) {
                std::fprintf(stderr, "[AssetManager] 加载失败: %s\n", path);
            }
        }
    }
    // 头像自动扫描并做圆形裁剪
    {
        avatarTex_.clear();
        namespace fs = std::filesystem;
        std::error_code ec;
        std::vector<std::string> paths;
        if (fs::exists("assets/ui/avatars", ec)) {
            for (auto& entry : fs::directory_iterator("assets/ui/avatars", ec)) {
                if (!entry.is_regular_file(ec)) continue;
                if (entry.path().extension() != ".png") continue;
                paths.push_back(entry.path().string());
            }
        }
        std::sort(paths.begin(), paths.end());
        for (const std::string& p : paths) {
            sf::Image img;
            if (!img.loadFromFile(p)) {
                std::fprintf(stderr, "[AssetManager] 加载失败: %s\n", p.c_str());
                continue;
            }
            cropToCircle(img);
            sf::Texture t;
            if (t.loadFromImage(img)) {
                avatarTex_.push_back(std::move(t));
            }
        }
    }
    // 桌面小贴图按文件名索引
    {
        tableTex_.clear();
        namespace fs = std::filesystem;
        std::error_code ec;
        if (fs::exists("assets/ui/table", ec)) {
            for (auto& entry : fs::directory_iterator("assets/ui/table", ec)) {
                if (!entry.is_regular_file(ec)) continue;
                if (entry.path().extension() != ".png") continue;
                sf::Texture t;
                if (t.loadFromFile(entry.path().string())) {
                    tableTex_[entry.path().stem().string()] = std::move(t);
                } else {
                    std::fprintf(stderr, "[AssetManager] 加载失败: %s\n",
                                 entry.path().string().c_str());
                }
            }
        }
    }

    return true;
}

const sf::Texture* AssetManager::cardTexture(Suit s, Rank r) const {
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

const sf::Texture* AssetManager::chipForAmount(int amount) const {
    for (const ChipDef& d : chipDefs_) {
        if (amount >= d.lo && amount < d.hi) {
            return d.tex.getSize().x > 0 ? &d.tex : nullptr;
        }
    }
    if (!chipDefs_.empty()) {
        return chipDefs_.front().tex.getSize().x > 0 ? &chipDefs_.front().tex : nullptr;
    }
    return nullptr;
}

void AssetManager::rollBack() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 2);
    backRoll_ = dist(rng);
}

const sf::Texture* AssetManager::icon(const std::string& name) const {
    auto it = icons_.find(name);
    if (it == icons_.end()) return nullptr;
    return it->second.getSize().x > 0 ? &it->second : nullptr;
}

const sf::Texture* AssetManager::deckPile(int backIndex) const {
    if (backIndex < 0 || backIndex > 2) backIndex = 0;
    return pileTex_[backIndex].getSize().x > 0 ? &pileTex_[backIndex] : nullptr;
}

const sf::Texture* AssetManager::avatarTexture(int idx) const {
    if (idx == 0 && customTex_.getSize().x > 0) return &customTex_;
    if (avatarTex_.empty()) return nullptr;
    int n = static_cast<int>(avatarTex_.size());
    int i = ((idx % n) + n) % n;
    return avatarTex_[i].getSize().x > 0 ? &avatarTex_[i] : nullptr;
}

const sf::Texture* AssetManager::customAvatar() const {
    return customTex_.getSize().x > 0 ? &customTex_ : nullptr;
}

bool AssetManager::reloadCustomAvatar() {
    // 缺失时静默返回, 避免 SFML 向 stderr 打告警
    if (!std::filesystem::exists("game_data/avatar.png")) {
        customTex_ = sf::Texture();
        return false;
    }
    sf::Image img;
    if (!img.loadFromFile("game_data/avatar.png")) {
        customTex_ = sf::Texture();
        return false;
    }
    cropToCircle(img);
    sf::Texture t;
    if (!t.loadFromImage(img)) {
        customTex_ = sf::Texture();
        return false;
    }
    customTex_ = std::move(t);
    return true;
}

const sf::Texture* AssetManager::tableTexture(const std::string& name) const {
    auto it = tableTex_.find(name);
    if (it == tableTex_.end()) return nullptr;
    return it->second.getSize().x > 0 ? &it->second : nullptr;
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
    return bgTex_.getSize().x > 0 ? &bgTex_ : nullptr;
}

const sf::Texture* AssetManager::buttonTexture(int state) const {
    if (state < 0 || state > 3) return nullptr;
    return btnTex_[state].getSize().x > 0 ? &btnTex_[state] : nullptr;
}
