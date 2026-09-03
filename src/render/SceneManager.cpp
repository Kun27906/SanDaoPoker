#include "render/SceneManager.h"
#include "core/Room.h"
#include "render/SceneMenu.h"
#include "render/SceneArrange.h"
#include "render/SceneBattle.h"
#include "render/SceneResult.h"
#include "render/SoundManager.h"

SceneManager::SceneManager(sf::RenderWindow& window) : window_(window) {
    current_ = createScene(SceneId::Menu);
}

void SceneManager::changeTo(SceneId id) {
    if (id == currentId_ && current_) return;
    currentId_ = id;
    current_ = createScene(id);
    // 场景切换音效:进组牌=发牌声,进比牌=翻牌声,进结算=筹码声
    switch (id) {
        case SceneId::Arrange: SoundManager::instance().playDeal(); break;
        case SceneId::Battle:  SoundManager::instance().playFlip(); break;
        case SceneId::Result:  SoundManager::instance().playChip(); break;
        default: break;
    }
    // BGM:主菜单播菜单曲,游戏场景播对局曲(当前同曲,后续可换)
    if (id == SceneId::Menu) {
        SoundManager::instance().playBgmMenu();
    } else {
        SoundManager::instance().playBgmGame();
    }
}

std::unique_ptr<Scene> SceneManager::createScene(SceneId id) {
    switch (id) {
        case SceneId::Menu:    return std::make_unique<SceneMenu>(this);
        case SceneId::Arrange: return std::make_unique<SceneArrange>(this);
        case SceneId::Battle:  return std::make_unique<SceneBattle>(this);
        case SceneId::Result:  return std::make_unique<SceneResult>(this);
    }
    return std::make_unique<SceneMenu>(this);
}

void SceneManager::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    if (current_) current_->handleEvent(e, win);
}

void SceneManager::update(float dt) {
    if (current_) current_->update(dt);
}

void SceneManager::draw(sf::RenderWindow& win) {
    if (current_) current_->draw(win);
}
