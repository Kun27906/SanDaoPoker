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
    // 筹码图标(区间命名,自动扫描 assets/ui/chips/chip_*.png)
    //   格式: chip_<min>-<max>.png | chip_-<max>.png(无下界) | chip_<min>-.png(无上界)
    {
        chipDefs_.clear();
        namespace fs = std::filesystem;
        std::error_code ec;
        if (fs::exists("assets/ui/chips", ec)) {
            for (auto& entry : fs::directory_iterator("assets/ui/chips", ec)) {
                if (!entry.is_regular_file(ec)) continue;
                if (entry.path().extension() != ".png") continue;
                std::string stem = entry.path().stem().string();   // "chip_-1000"
                if (stem.rfind("chip_", 0) != 0) continue;
                std::string range = stem.substr(5);                // "-1000" / "1000-2000" / "15000-"
                int lo = INT_MIN, hi = INT_MAX;
                std::size_t dash = range.find('-');
                if (dash == std::string::npos) {
                    lo = hi = std::atoi(range.c_str());
                } else {
                    std::string a = range.substr(0, dash);   // 空=无下界
                    std::string b = range.substr(dash + 1);  // 空=无上界
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
    // 界面图标(自动扫描 assets/ui/icons/*.png, 文件名即查询名)
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
    // 牌堆素材(发牌动画, 三色 689x292 堆叠图)
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
    // 头像素材: 自动扫描 assets/ui/avatars/*.png, 加载后按"内切圆"做 alpha 圆形裁剪
    //   → 得到真正的圆形头像(与 GitHub 头像一致), 方形图四角被切除(非遮挡)
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
            // ---- 圆形裁剪: 距中心 > 半径的像素 alpha 置 0(边缘 1px 羽化抗锯齿) ----
            const unsigned w = img.getSize().x, h = img.getSize().y;
            const float ccx = (w - 1) * 0.5f, ccy = (h - 1) * 0.5f;
            const float rad = std::min(w, h) * 0.5f;
            for (unsigned y = 0; y < h; y++) {
                for (unsigned x = 0; x < w; x++) {
                    float dx = static_cast<float>(x) - ccx;
                    float dy = static_cast<float>(y) - ccy;
                    float d = std::sqrt(dx * dx + dy * dy);
                    float inside = rad - d;             // >0 = 圆内
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
            sf::Texture t;
            if (t.loadFromImage(img)) {
                avatarTex_.push_back(std::move(t));
            }
        }
    }
    // 桌面小贴图(assets/ui/table/*.png 自动扫描, 文件名(去扩展名)即查询名)
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

const sf::Texture* AssetManager::chipForAmount(int amount) const {
    // 区间规则: lo <= amount < hi (下含上不含); 找到第一个命中区间
    for (const ChipDef& d : chipDefs_) {
        if (amount >= d.lo && amount < d.hi) {
            return d.tex.getSize().x > 0 ? &d.tex : nullptr;
        }
    }
    // 兜底:返回第一张(理论不可达,区间应覆盖全值域)
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
    if (avatarTex_.empty()) return nullptr;
    int n = static_cast<int>(avatarTex_.size());
    int i = ((idx % n) + n) % n;                  // 取模循环
    return avatarTex_[i].getSize().x > 0 ? &avatarTex_[i] : nullptr;
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
    return bgTex_.getSize().x > 0 ? &bgTex_ : nullptr;  // 回退到桌面背景
}

const sf::Texture* AssetManager::buttonTexture(int state) const {
    if (state < 0 || state > 3) return nullptr;
    return btnTex_[state].getSize().x > 0 ? &btnTex_[state] : nullptr;
}
