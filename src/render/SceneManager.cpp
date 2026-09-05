#include "render/SceneManager.h"
#include "core/Room.h"
#include "render/SceneTitle.h"
#include "render/SceneLobby.h"
#include "render/SceneRoomSelect.h"
#include "render/SceneArrange.h"
#include "render/SceneBattle.h"
#include "render/SceneResult.h"

SceneManager::SceneManager(sf::RenderWindow& window) : window_(window) {
    current_ = createScene(currentId_);
}

void SceneManager::changeTo(SceneId id) {
    current_ = createScene(id);
    currentId_ = id;
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
