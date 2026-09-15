#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "core/Room.h"
#include "render/GlobalHud.h"
#include "render/SceneSetup.h"

enum class SceneId {
    Title,       // 第一界面: 启动页
    Lobby,       // 第二界面: 大厅
    RoomSelect,  // 第三界面: 选房间
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
    // 全局左上角 home 键按下
    virtual void onHomePressed() {}
};

class SceneManager {
public:
    explicit SceneManager(sf::RenderWindow& window);

    // 切换到指定场景
    void changeTo(SceneId id);
    SceneId currentId() const { return currentId_; }

    void handleEvent(const sf::Event& e, const sf::RenderWindow& win);
    void update(float dt);
    void draw(sf::RenderWindow& win);

    bool homeVisible() const;   // 组牌/比牌/启动页隐藏
    void onHomePressed();       // 转发给当前场景

    // 返回 true = 已拦截, 调用方不要关窗口
    // 返回 false = 可直接退出
    bool onCloseRequested(bool isEscape);
    bool shouldClose() const;   // true = 玩家已在确认窗点[确定] -> 关闭窗口

    std::unique_ptr<Room> room;   // 当前房间
    int selectedPlayerCount = 4;  // 大厅选的人数,决定房间列表

private:
    std::unique_ptr<Scene> createScene(SceneId id);

    sf::RenderWindow& window_;
    std::unique_ptr<Scene> current_;
    SceneId currentId_ = SceneId::Title;
    std::unique_ptr<GlobalHud> hud_;   // 全局悬浮工具栏
};
