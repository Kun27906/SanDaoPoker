# SanDaoPoker（炸金花三道）

单机版炸金花三道游戏（1 真人 + AI），C++17 + SFML 2.6 + CMake 构建。

## 四人分工

| 成员 | 模块 | 负责目录 |
|------|------|----------|
| A | 核心逻辑（牌/牌型/回合） | `include/core/` `src/core/` |
| B | AI 玩家（分组搜索/难度） | `include/ai/` `src/ai/` |
| C | 渲染与交互（界面） | `include/render/` `include/ui/` `src/render/` `src/ui/` |
| D | 素材/构建/集成（项目经理） | `assets/` `CMakeLists.txt` `third_party/` |

头文件与实现分离：`.h` 统一放 `include/<模块>/`，`.cpp` 放 `src/<模块>/`，
源码中按 `#include "core/Card.h"` 方式引用。
依赖约定：逻辑层（`src/core`）不包含任何 SFML 头文件，可独立单元测试；渲染层只调用逻辑层接口。

## 分支约定

- `main` — 正式版（稳定，可运行）
- `A` `B` `C` `D` — 各成员工作分支，每天 push，由 D 合并回 main

## 协作流程

1. 每天开工前 `git pull` 拉最新
2. 在自己分支开发，提交信息写明改动内容
3. 下班前 push 到自己的分支
4. D 每日合并到 main，解决冲突后通知全员

## 目录结构

```
SanDaoPoker/
├── .gitignore              # Git 忽略规则
├── CMakeLists.txt          # 顶层构建脚本
├── README.md
├── docs/                   # 项目文档（规则文档等）
├── third_party/            # 第三方库（SFML-2.6.1.zip 随仓库分发，CMake 自动解压）
├── assets/                 # 游戏资源
│   ├── cards/              # 扑克牌贴图（55 张牌 + 3 种牌背）
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
└── tools/                  # 工具脚本
```

## 构建方法（Windows）

### 需要的工具

1. **Visual Studio 2022 Community**（免费）— 安装时勾选 **"使用 C++ 的桌面开发"**
2. **CMake** — VS 安装器里勾选，或到 cmake.org 下载
3. **Git**（Git Bash）— git-scm.com 下载
4. SFML 2.6 已内置（`third_party/SFML-2.6.1.zip`，CMake 首次构建自动解压，无需手动安装）

### 编译步骤

```bash
git clone git@github.com:Kun27906/SanDaoPoker.git
cd SanDaoPoker
cmake -B build -G Ninja
cmake --build build
```

（若未安装 Ninja，去掉 `-G Ninja` 使用 VS 默认生成器即可；
首次构建会自动解压 SFML 并拷贝 DLL 到 exe 旁，双击就能跑。）

### 运行

```
build\SanDaoPoker.exe   （或 VS 里点"运行"）
```

## 当前状态

工程框架已就绪（目录结构、构建脚本、SFML 自动解压、资源目录、骨架源码），
源码按《docs/项目游戏规则.docx》与四人分工方案逐步实现。
