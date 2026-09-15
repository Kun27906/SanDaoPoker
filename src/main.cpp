// 流程: 锚定工作目录 -> 加载本地存档 -> 加载素材 -> 启动窗口与场景
#include "render/Account.h"
#include "render/AssetManager.h"
#include "render/GameApp.h"

#include <filesystem>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

static void anchorWorkingDir() {
    // 工作目录锚定到 exe 所在目录, 素材与存档相对路径不随启动方式变化
    std::wstring path(MAX_PATH, L'\0');
    for (;;) {
        const DWORD len = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
        if (len == 0) return;
        if (len < path.size()) {
            path.resize(len);
            break;
        }
        path.resize(path.size() * 2);
    }
    std::error_code ec;
    std::filesystem::current_path(std::filesystem::path(path).parent_path(), ec);
}

int main() {
    anchorWorkingDir();
    Account::instance().load();
    AssetManager::instance().loadAll();
    GameApp app;
    app.run();
    return 0;
}
