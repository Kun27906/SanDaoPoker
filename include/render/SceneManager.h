#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "core/Room.h"   // 完整类型(unique_ptr<Room> 析构需要)

enum class SceneId {
    Title,       // 第一界面: 启动页(开始游戏)
    Lobby,       // 第二界面: 大厅(按人数 2~6 选)
    RoomSelect,  // 第三界面: 选房间(按所选人数过滤)
    Arrange,     // 组牌界面(阶段5:分三道+倒计时)
    Battle,      // 比牌界面(阶段6:翻牌动画)
    Result       // 结算界面(阶段7:筹码/盈亏)
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

    // ---- 游戏会话状态(跨场景共享) ----
    std::unique_ptr<Room> room;   // 当前房间(选房后创建,组牌/比牌/结算使用)
    int selectedPlayerCount = 4;  // 大厅选的人数(2~6),决定房间列表

private:
    std::unique_ptr<Scene> createScene(SceneId id);

    sf::RenderWindow& window_;
    std::unique_ptr<Scene> current_;
    SceneId currentId_ = SceneId::Title;
};
