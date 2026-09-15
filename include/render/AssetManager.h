#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <vector>
#include "core/Card.h"

// 素材单例: 牌面 牌背 大小王 背景 按钮 筹码 图标
class AssetManager {
public:
    static AssetManager& instance();

    bool loadAll();

    // 牌面纹理, 大小王或加载失败返回 nullptr
    const sf::Texture* cardTexture(Suit s, Rank r) const;
    // 牌背: 0 红 1 蓝 2 黑
    const sf::Texture* backTexture(int index = 0) const;
    const sf::Texture* background() const;
    // 主菜单背景, 缺失时回落桌面背景
    const sf::Texture* menuBackground() const;
    // 按钮四态: 0 normal 1 hover 2 pressed 3 disabled
    const sf::Texture* buttonTexture(int state) const;
    const sf::Texture* chipForAmount(int amount) const;
    void rollBack();
    int currentBack() const { return backRoll_; }
    const sf::Texture* icon(const std::string& name) const;
    // 牌堆堆叠图, 按牌背色取
    const sf::Texture* deckPile(int backIndex) const;
    // 头像纹理, 0 号优先返回自定义头像
    const sf::Texture* avatarTexture(int idx) const;
    const sf::Texture* customAvatar() const;
    bool reloadCustomAvatar();
    const sf::Texture* tableTexture(const std::string& name) const;

private:
    AssetManager() = default;
    bool loadCardTextures();
    bool loadMiscTextures();

    sf::Texture cardTex_[4][13];
    sf::Texture jokerTex_[2];
    std::vector<sf::Texture> backTex_;
    sf::Texture bgTex_;
    sf::Texture menuTex_;
    sf::Texture btnTex_[4];
    // 筹码按区间命名
    struct ChipDef { int lo; int hi; sf::Texture tex; };
    std::vector<ChipDef> chipDefs_;
    std::map<std::string, sf::Texture> icons_;
    sf::Texture pileTex_[3];
    std::vector<sf::Texture> avatarTex_;
    sf::Texture customTex_;
    std::map<std::string, sf::Texture> tableTex_;
    int backRoll_ = 0;
    bool loaded_ = false;
};
