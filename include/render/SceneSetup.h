#pragma once
#include <SFML/Graphics.hpp>
#include "render/AssetManager.h"
#include "render/Layout.h"
#include "render/Account.h"
#include "ui/Avatar.h"
#include "ui/ChipBar.h"
#include "ui/VersionBadge.h"

// 场景公共设置: 背景铺满 筹码条右上 版本号左下 本人头像
namespace scene_setup {

inline void background(sf::Sprite& sp, const sf::Texture* tex, float w, float h) {
    if (!tex) return;
    sp.setTexture(*tex);
    sp.setScale(w / tex->getSize().x, h / tex->getSize().y);
}

inline void chipBar(ChipBar& bar, float windowW) {
    bar.setPosition(sf::Vector2f(windowW - 270.f, 16.f));
}

inline void versionBadge(VersionBadge& badge, float windowH) {
    badge.setPosition(sf::Vector2f(24.f, windowH - 40.f));
}

inline void selfAvatar(Avatar& av, float windowW) {
    av.setRadius(26.f);
    av.setMinPlateWidth(150.f);
    av.setSelfStyle(true);
    av.setTexture(AssetManager::instance().avatarTexture(0));
    av.setCenter(sf::Vector2f(windowW - 460.f, 40.f));
    av.setNickname(Account::instance().ensureNickname());
}

}
