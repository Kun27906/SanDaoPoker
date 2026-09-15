#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "core/Room.h"
#include "render/GlobalHud.h"
#include "render/SceneSetup.h"

enum class SceneId {
    Title,
    Lobby,
    RoomSelect,
    Deal,
    Arrange,
    Battle,
    Result
};

class Scene {
public:
    virtual ~Scene() = default;
    virtual void handleEvent(const sf::Event& e, const sf::RenderWindow& win) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& win) = 0;
    virtual void onHomePressed() {}
};

class SceneManager {
public:
    explicit SceneManager(sf::RenderWindow& window);

    void changeTo(SceneId id);
    SceneId currentId() const { return currentId_; }

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void update(float dt);
    void draw(sf::RenderWindow& win);

    bool homeVisible() const;
    void onHomePressed();

    bool onCloseRequested(bool isEscape);
    bool shouldClose() const;

    std::unique_ptr<Room> room;
    int selectedPlayerCount = 4;

private:
    std::unique_ptr<Scene> createScene(SceneId id);

    sf::RenderWindow& window_;
    std::unique_ptr<Scene> current_;
    SceneId currentId_ = SceneId::Title;
    std::unique_ptr<GlobalHud> hud_;
};
