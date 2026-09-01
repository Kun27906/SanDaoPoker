# 炸金花三道 (SanDaoPoker)

单机版扑克游戏:C++17 + SFML 2.6 + CMake · 1真人 + AI

## 四人分工

| 成员 | 模块 | 负责目录 |
|---|---|---|
| A | 核心逻辑(牌/牌型/回合) | `src/core/` |
| B | AI玩家(分组搜索/难度) | `src/ai/` |
| C | 渲染与交互(界面) | `src/render/` `src/ui/` |
| D | 素材/构建/集成(项目经理) | `assets/` `CMakeLists.txt` `third_party/` |

## 分支约定

- `main` — 正式版(稳定,可运行)
- `A` `B` `C` `D` — 各成员工作分支,每天 push,由 D 合并回 main

## 协作流程

1. 每天开工前 `git pull` 拉最新
2. 在自己分支开发,提交信息写明改动内容
3. 下班前 push 到自己的分支
4. D 每日合并到 main,解决冲突后通知全员

## 构建方法(Windows)

### 需要的工具
1. **Visual Studio 2022 Community**(免费)— 安装时勾选 **"使用 C++ 的桌面开发"**
2. **CMake** — VS 安装器里勾选,或到 cmake.org 下载
3. **SFML 2.6**(渲染库)— 暂时没有也能编译纯 C++ 骨架

### 编译步骤
```bash
git clone git@github.com:Kun27906/SanDaoPoker.git
cd SanDaoPoker
cmake -B build
cmake --build build
```
或用 VS:打开文件夹 → 选 SanDaoPoker → VS 自动识别 CMakeLists → 生成 → 全部生成

### 运行
```
build\SanDaoPoker.exe   (或 VS 里点"运行")
```
