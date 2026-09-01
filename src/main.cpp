// SanDaoPoker 程序入口(成员C,阶段2/3:素材加载 + 场景状态机)
#include "render/AssetManager.h"
#include "render/GameApp.h"

int main() {
    // 先加载素材,再启动窗口
    AssetManager::instance().loadAll();
    GameApp app;
    app.run();
    return 0;
}
