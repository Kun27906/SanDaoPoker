# SanDaoPoker（炸金花三道）

单机版炸金花三道游戏：每桌 1 名真人玩家与 1~5 名 AI 对战（2~6 人房）。C++17 + SFML 2.6 + CMake 构建，运行于 Windows。

## 模块分工

| 成员 | 模块 | 负责目录 |
|------|------|----------|
| A | 核心逻辑（牌/牌型/回合） | `include/core/` `src/core/` |
| B | AI 玩家（分组搜索/难度） | `include/ai/` `src/ai/` |
| C | 渲染与交互（界面） | `include/render/` `include/ui/` `src/render/` `src/ui/` |
| D | 素材/构建/集成 | `assets/` `CMakeLists.txt` `third_party/` |

头文件与实现分离：`.h` 统一放 `include/<模块>/`，`.cpp` 放 `src/<模块>/`，源码中按 `#include "core/Card.h"` 方式引用。
依赖约定：逻辑层（`src/core`）不包含任何 SFML 头文件，可独立单元测试；渲染层只调用逻辑层接口。

## 分支约定

- `main` — 稳定版本
- `A` `B` `C` `D` — 成员工作分支；开发与合并流程见 `docs/Git团队协作指南.md`

## 构建（Windows）

需要安装：

1. **Visual Studio 2022 Community** — 安装时勾选“使用 C++ 的桌面开发”
2. **CMake** — VS 安装器内勾选，或从 cmake.org 下载
3. **Git** — git-scm.com 下载

一键构建：双击 `tools\build_all.bat`，自动定位 Visual Studio 环境并完成配置与编译。

手动构建（在已加载 MSVC 环境的终端中执行）：

```bash
git clone git@github.com:Kun27906/SanDaoPoker.git
cd SanDaoPoker
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

首次构建自动解压 SFML 并同步素材；构建产物集中输出到 `bin/`，完整说明见 `docs/重新编译流程.md`。

## 运行

```
bin\SanDaoPoker.exe
```

exe、SFML DLL、`assets/` 素材与 `game_data/` 存档均位于 `bin/`；程序启动时自动以自身所在目录为工作目录，可从任意路径启动。

## 目录结构

```
SanDaoPoker/
├── .gitignore              # Git 忽略规则
├── CMakeLists.txt          # 顶层构建脚本
├── README.md
├── docs/                   # 项目文档（规则/构建/协作）
├── third_party/            # 第三方库（SFML-2.6.1.zip 随仓库分发，CMake 自动解压）
├── assets/                 # 游戏资源（详见 assets/README.md）
│   ├── cards/              # 扑克牌贴图
│   ├── ui/                 # 界面素材（背景/按钮/筹码/图标/桌面）
│   ├── fonts/              # 字体文件
│   └── sounds/             # 音效与背景音乐
├── include/                # 公共头文件（与实现分离）
│   ├── core/               # 核心逻辑接口（无 SFML 依赖）
│   ├── ai/                 # AI 玩家接口
│   ├── render/             # 渲染与交互接口
│   └── ui/                 # 通用控件接口
├── src/                    # 实现文件
│   ├── main.cpp            # 程序入口
│   ├── core/               # 核心逻辑实现
│   ├── ai/                 # AI 玩家实现
│   ├── render/             # 渲染与交互实现
│   └── ui/                 # 通用控件实现
├── tests/                  # 单元测试
└── tools/                  # 开发工具（一键构建脚本、素材生成脚本）
```

## 当前状态

已完成完整对局流程：大厅、选房、下注、发牌、组牌、比牌、结算；含本地账户存档、AI 难度分级、随机与自设昵称/头像、音效系统、逃跑与踢出规则。版本记录见 `include/core/VersionInfo.h`。

游戏规则见 `docs/项目游戏规则.docx`。
