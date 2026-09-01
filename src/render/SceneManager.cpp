#include "render/SceneManager.h"
#include "core/Room.h"
#include "render/SceneMenu.h"
#include "render/SceneArrange.h"
#include "render/SceneBattle.h"
#include "render/SceneResult.h"

SceneManager::SceneManager(sf::RenderWindow& window) : window_(window) {
    current_ = createScene(SceneId::Menu);
}

void SceneManager::changeTo(SceneId id) {
    if (id == currentId_ && current_) return;
    currentId_ = id;
    current_ = createScene(id);
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
