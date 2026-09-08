#include "render/SceneManager.h"
#include "render/SoundManager.h"
#include "core/Room.h"
#include "render/SceneTitle.h"
#include "render/SceneLobby.h"
#include "render/SceneRoomSelect.h"
#include "render/SceneArrange.h"
#include "render/SceneBattle.h"
#include "render/SceneResult.h"

SceneManager::SceneManager(sf::RenderWindow& window) : window_(window) {
    hud_ = std::make_unique<GlobalHud>(this);
    current_ = createScene(currentId_);
}

void SceneManager::changeTo(SceneId id) {
    current_ = createScene(id);
    currentId_ = id;
    // 背景音乐随场景切换(仅 bgmOn 时播放;同一曲不重播)
    switch (id) {
        case SceneId::Arrange:
        case SceneId::Battle:
        case SceneId::Result:
            SoundManager::instance().playBgmGame();
            break;
        default:
            SoundManager::instance().playBgmMenu();
            break;
    }
}

void SceneManager::handleEvent(const sf::Event& e, const sf::RenderWindow& win) {
    // 全局工具栏先处理;弹窗打开时返回 true 拦截(不穿透到场景)
    if (hud_ && hud_->handleEvent(e, win)) return;
    if (current_) current_->handleEvent(e, win);
}

void SceneManager::update(float dt) {
    if (current_) current_->update(dt);
}

void SceneManager::draw(sf::RenderWindow& win) {
    if (current_) current_->draw(win);
    if (hud_) hud_->draw(win);   // 全局工具栏+弹窗在最上层
}

bool SceneManager::homeVisible() const {
    // 启动页/组牌/比牌隐藏 home; 大厅/选房/结算显示
    return currentId_ != SceneId::Title &&
           currentId_ != SceneId::Arrange &&
           currentId_ != SceneId::Battle;
}

void SceneManager::onHomePressed() {
    if (current_) current_->onHomePressed();
}

std::unique_ptr<Scene> SceneManager::createScene(SceneId id) {
    switch (id) {
        case SceneId::Title:      return std::make_unique<SceneTitle>(this);
        case SceneId::Lobby:      return std::make_unique<SceneLobby>(this);
        case SceneId::RoomSelect: return std::make_unique<SceneRoomSelect>(this);
        case SceneId::Arrange:    return std::make_unique<SceneArrange>(this);
        case SceneId::Battle:     return std::make_unique<SceneBattle>(this);
        case SceneId::Result:     return std::make_unique<SceneResult>(this);
    }
    return std::make_unique<SceneTitle>(this);
}
