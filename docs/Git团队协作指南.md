# Git 团队协作指南

适用：SanDaoPoker 开发组（成员 A/B/C/D）
仓库：`github.com/Kun27906/SanDaoPoker`（分支：main + A/B/C/D）

## 〇、术语速查

| 词 | 含义 |
|---|---|
| 仓库 Repository | 项目的云端目录，记录每一次改动的历史 |
| 克隆 clone | 把云端仓库完整下载到本地，仅首次需要 |
| 提交 commit | 把改动记录到本地历史 |
| 推送 push | 把本地提交上传到云端 |
| 拉取 pull | 把云端改动下载并合入本地 |
| 分支 branch | 独立工作线：A/B/C/D 各自开发、互不干扰 |
| 合并 merge | 把某条分支的改动并入另一条分支 |
| 冲突 conflict | 两名成员改动了同一处，Git 无法自动合并，需人工取舍 |

核心顺序：先 pull，再修改，再 commit，最后 push。

## 一、首次准备（每人一次）

### 1. 安装工具

- **Visual Studio 2022 Community**：安装时勾选“使用 C++ 的桌面开发”（自带 CMake、Ninja）
- **Git**：从 git-scm.com 下载，默认选项安装（自带 Git Bash）

### 2. 配置 SSH（免密推送）

在 Git Bash 中执行（Windows 可右键桌面 → “Git Bash Here”）：

```bash
# ① 生成密钥（回车使用默认值，邮箱替换为本人邮箱）
ssh-keygen -t ed25519 -C "邮箱@example.com"

# ② 查看公钥并复制
cat ~/.ssh/id_ed25519.pub
```

打开 GitHub → 头像 → **Settings → SSH and GPG keys → New SSH key** → 粘贴公钥 → Add SSH key。
验证（出现 `Hi <用户名>!` 即成功）：

```bash
ssh -T git@github.com
```

若网络无法连接 GitHub 的 22 端口（公司/校园网常见），在 `~/.ssh/config` 中添加：

```
Host github.com
  HostName ssh.github.com
  Port 443
  User git
```

### 3. 克隆仓库并切换成员分支

```bash
git clone git@github.com:Kun27906/SanDaoPoker.git
cd SanDaoPoker

# 成员A执行 checkout A，成员B执行 B，依此类推（D 留在 main 做集成）
git checkout A
```

成员分支相互独立，日常改动不会影响他人。

## 二、每日工作流

```bash
# ① 开工：拉取最新
git pull

# ② 修改代码/文件

# ③ 查看改动（红=删除/修改，绿=新增）
git status

# ④ 暂存改动
git add -A

# ⑤ 提交（说明改动内容）
git commit -m "实现HandEvaluator豹子牌型判定+单元测试"

# ⑥ 推送到自己的分支
git push
```

提交信息规范：一句话写明“做了什么 + 为什么”。示例：`实现牌型比较:豹子>同花顺>金花,含异花235特殊规则`；不使用 `update`、`修改`、`111` 等无信息量描述。

## 三、合并到 main（由成员 D 负责）

```bash
git checkout main
git pull                            # 确保 main 最新
git merge A                         # 合并 A 的改动（B/C/D 同理）
# 如出现冲突：见“四、冲突处理”
git push
git checkout D                      # 切回工作分支
```

其他成员不直接 push 到 main，统一由 D 合并。

## 四、冲突处理

冲突 = 两名成员改动了同一文件的同一处，Git 无法自动取舍。

出现 `CONFLICT` 提示时：

```bash
# ① 查看冲突文件
git status                # 标注 both modified 的即为冲突文件

# ② 用编辑器打开冲突文件，定位以下标记：
# <<<<<<< HEAD
#   本分支代码
# =======
#   合入分支代码
# >>>>>>> A

# ③ 保留需要的版本（可保留双方），删除 <<<<<<< ======= >>>>>>> 三行标记

# ④ 保存后提交
git add -A
git commit -m "解决与A分支的冲突:保留双方牌型定义"
git push
```

减少冲突的做法：每日开工先 `git pull`；各成员仅修改自己模块的文件（A 改 core、B 改 ai、C 改 render/ui）。

## 五、常见问题速查

| 问题 | 原因 | 解决 |
|---|---|---|
| `Permission denied (publickey)` | SSH 公钥未添加或添加到了其他账号 | 重新执行“配置 SSH”第②步；确认公钥添加在有仓库权限的账号 |
| push 提示 `failed to push some refs` | 远端有新提交，本地落后 | `git pull` 后重新 `git push` |
| push 提示 `Permission to xxx denied` | 当前账号不是仓库协作者 | 由仓库所有者添加为 Collaborators（Write 权限） |
| `git pull` 报“本地改动会被覆盖” | 存在未提交的修改 | 先 `git commit` 提交，或 `git stash` 暂存后再 pull |
| 需要撤销未提交的改动 | — | `git checkout -- 文件名` 或 `git restore 文件名` |
| `git add` 误加了文件 | — | `git reset HEAD 文件名`（移出暂存区，改动保留） |
| 提交信息写错 | — | `git commit --amend` 重新编辑（仅限未推送时） |
| 误提交到 main | — | `git reset --soft HEAD~1` 撤回提交，切回成员分支重新提交 |
| 撤销最近一次提交但保留改动 | — | `git reset --soft HEAD~1` |
| 彻底删除最近一次提交及改动 | — | `git reset --hard HEAD~1`（危险：改动不可恢复） |
| 成员分支落后 main 较多 | main 已有新合并 | 在成员分支执行 `git fetch` → `git merge origin/main`（冲突按“四、冲突处理”） |
| 中途需要切换任务 | — | `git stash` 暂存，处理完毕后 `git stash pop` 恢复 |
| 找回已提交过的文件 | — | `git checkout -- 文件名`，或 `git checkout <commit> -- 文件名` |
| 查看项目历史 | — | `git log --oneline`（含分支图：`--graph`） |
| 查看某文件的修改记录 | — | `git log -- <文件名>` → `git show <commit>` |
| 远程分支已删除，本地仍存在 | 远端已清理 | `git fetch --prune` 同步；本地分支用 `git branch -d 分支名` 删除 |
| push 要求输入用户名密码 | 使用 HTTPS 方式 | 改用 SSH（重新 clone 或 `git remote set-url origin git@github.com:...`） |
| 编译报头文件缺失 | 本地代码落后于接口改动 | `git pull` 拉取最新；接口改动需提前在团队内报备 |
| 素材/画面陈旧 | build 内为旧快照 | 见 `docs/重新编译流程.md`：删除 build 目录重新编译 |

## 六、团队约定

1. 每日开工第一件事为 `git pull`，下班前完成 `git push`
2. 仅 push 自己的成员分支（A→A、B→B…），main 由 D 合并
3. 提交信息格式：`实现X功能/修复Y问题 + 一句话说明`
4. 仅修改自己模块的文件；跨模块改动先与相关成员同步
5. 修改接口（头文件）必须先报备，避免其他成员编译失败
6. 本地构建与测试通过后再提交（`cmake --build build`）
7. 无法解决的问题在团队群内反馈

## 七、构建命令（Windows）

一键构建：双击 `tools\build_all.bat`。
手工构建（在已加载 MSVC 环境的终端中）：

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release   # 首次自动解压 third_party/SFML-2.6.1.zip
cmake --build build
```

单元测试：

```bash
cmake -B build -G Ninja -DBUILD_TESTS=ON
build\test_hand_evaluator.exe
```

未覆盖的问题：反馈时提供完整报错原文。
