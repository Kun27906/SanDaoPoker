#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "core/Card.h"

// ====== AssetManager 资源管理器(阶段2) ======
// 单例,统一加载和管理游戏素材:
//   - 扑克牌贴图 52 张:  assets/cards/{spades,hearts,clubs,diamonds}/{A,2..10,J,Q,K}.png
//     (命名约定见 assets/README.md:文件名即点数名,如 A.png/10.png/J.png/Q.png/K.png)
//   - 大小王 2 张:       assets/cards/Jokers/{big,small}.png
//   - 牌背 3 张:         assets/cards/back/{red,blue,black}.png
//   - 桌面背景:          assets/ui/backgrounds/table_bg.png
//   - 按钮四态图:        assets/ui/buttons/btn_{normal,hover,pressed,disabled}.png
// 用法:
//   AssetManager::instance().loadAll();        // 程序启动时调用一次
//   const sf::Texture* t = AssetManager::instance().cardTexture(suit, rank);
class AssetManager {
public:
    static AssetManager& instance();

    // 加载全部素材(可重复调用,内部有保护)
    bool loadAll();

    // ---- 查询接口 ----
    // 按 A 成员 Card 枚举取牌面纹理;大小王(无贴图)或加载失败返回 nullptr
    const sf::Texture* cardTexture(Suit s, Rank r) const;
    // 牌背纹理:0=红 1=蓝 2=黑(默认红)
    const sf::Texture* backTexture(int index = 0) const;
    // 桌面背景(游戏场景,1920x1080)
    const sf::Texture* background() const;
    // 主菜单背景(1920x1080),加载失败时回退到桌面背景
    const sf::Texture* menuBackground() const;
    // 按钮图:0=normal 1=hover 2=pressed 3=disabled
    const sf::Texture* buttonTexture(int state) const;

    bool isLoaded() const { return loaded_; }

private:
    AssetManager() = default;
    bool loadCardTextures();
    bool loadMiscTextures();

    // [花色][下标0..12] (0=2 ... 8=10, 9=J, 10=Q, 11=K, 12=A)
    sf::Texture cardTex_[4][13];
    sf::Texture jokerTex_[2];          // 大小王: [0]=small [1]=big
    std::vector<sf::Texture> backTex_;   // 3 张牌背
    sf::Texture bgTex_;                  // 桌面背景(游戏场景)
    sf::Texture menuTex_;                // 主菜单背景
    sf::Texture btnTex_[4];              // 按钮四态
    bool loaded_ = false;
};
