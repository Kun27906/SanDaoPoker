// SanDaoPoker 程序入口(成员C)
// 流程:加载本地存档(账号) -> 加载素材 -> 启动窗口与场景
#include "render/Account.h"
#include "render/AssetManager.h"
#include "render/GameApp.h"

int main() {
    Account::instance().load();      // 读取/初始化玩家账号存档(game_data/save.dat)
    AssetManager::instance().loadAll();
    GameApp app;
    app.run();
    return 0;
}
