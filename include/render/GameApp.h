#pragma once
#include <SFML/Graphics.hpp>
#include "render/SceneManager.h"

class GameApp {
public:
    GameApp();
    void run();

private:
    sf::RenderWindow window_;
    sf::Clock clock_;
    SceneManager sceneManager_;
};
