#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

// ====== 场景状态机(阶段3) ======
// 场景流: 菜单(Menu) -> 组牌(Arrange) -> 比牌(Battle) -> 结算(Result) -> 菜单
// Scene 为场景基类,四个场景继承并实现 handleEvent/update/draw。
// SceneManager 持有当前场景,负责切换(changeTo)。

enum class SceneId {
    Menu,      // 主菜单(阶段4:选房间)
    Arrange,   // 组牌界面(阶段5:分三道+倒计时)
    Battle,    // 比牌界面(阶段6:翻牌动画)
    Result     // 结算界面(阶段7:筹码/排名)
};

// ====== 场景基类 ======
class Scene {
public:
    virtual ~Scene() = default;
    virtual void handleEvent(const sf::Event& e, const sf::RenderWindow& win) = 0;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& win) = 0;
};

// ====== 场景管理器 ======
class SceneManager {
public:
    explicit SceneManager(sf::RenderWindow& window);

    // 切换到指定场景(旧场景销毁,新场景创建)
    void changeTo(SceneId id);
    SceneId currentId() const { return currentId_; }

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void update(float dt);
    void draw(sf::RenderWindow& win);

private:
    std::unique_ptr<Scene> createScene(SceneId id);

    sf::RenderWindow& window_;
    std::unique_ptr<Scene> current_;
    SceneId currentId_ = SceneId::Menu;
};
