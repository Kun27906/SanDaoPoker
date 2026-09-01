# Git 团队协作指南（新手版）

> 适用：SanDaoPoker 四人开发组（成员 A/B/C/D）
> 仓库：`github.com/Kun27906/SanDaoPoker`（分支：main + A/B/C/D）

---

## 〇、先搞懂几个词（白话版）

| 词 | 意思 |
|---|---|
| **仓库 (Repository)** | 项目的"云端文件夹"，记录每一次改动的历史 |
| **克隆 (clone)** | 把云端仓库完整下载到本地，第一次用 |
| **提交 (commit)** | 把你改的东西"拍照存档"（只存在本地） |
| **推送 (push)** | 把本地存档上传到云端（别人才能看到） |
| **拉取 (pull)** | 把云端别人的改动下载到本地（每天开工必做） |
| **分支 (branch)** | 项目的"平行世界"：A/B/C/D 各干各的互不干扰 |
| **合并 (merge)** | 把某个分支的改动合进另一个分支 |
| **冲突 (conflict)** | 两个人改了同一处，git 不知道听谁的，需要人工裁决 |

**铁律**：`pull` 是"下载"、`push` 是"上传"、`commit` 是"本地存档"。
**顺序永远是：先 pull，再干活，再 commit，最后 push。**

---

## 一、第一次准备（每人只做一次）

### 1. 安装工具
- **Visual Studio 2022 Community**：安装时勾选 **"使用 C++ 的桌面开发"**（自带 CMake、Ninja）
- **Git**：从 git-scm.com 下载安装，全程默认下一步（自带 Git Bash）

### 2. 配置 SSH（推荐，免密推送）
在 **Git Bash** 里执行（Windows 可右键桌面 → "Git Bash Here"）：

```bash
# ① 生成密钥（一路回车即可，邮箱换成自己的）
ssh-keygen -t ed25519 -C "你的邮箱@example.com"

# ② 查看公钥并复制
cat ~/.ssh/id_ed25519.pub
```

- 打开 GitHub → 头像 → **Settings → SSH and GPG keys → New SSH key** → 粘贴公钥 → Add SSH key
- 验证（出现 `Hi 你的用户名!` 即成功）：
  ```bash
  ssh -T git@github.com
  ```

> ⚠️ 若网络连不上 GitHub 的 22 端口（公司/校园网常见），在 `~/.ssh/config` 里加：
> ```
> Host github.com
>   HostName ssh.github.com
>   Port 443
>   User git
> ```

### 3. 克隆仓库 + 切自己的分支

```bash
git clone git@github.com:Kun27906/SanDaoPoker.git
cd SanDaoPoker

# 成员A执行 A，成员B执行 B，以此类推（D 留在 main 做集成）
git checkout A        # 或 B / C / D
```

> 你的分支就是你专属的工作区，随便改，不会影响别人。

---

## 二、每日工作流（每天重复）

```bash
# ① 开工：拉取最新（把别人昨天的改动拿下来）
git pull

# ② 写代码、改文件……

# ③ 查看改了什么（红=删/改，绿=新增）
git status

# ④ 把改动加入"暂存区"（全部改动）
git add -A

# ⑤ 提交（写清楚这次干了啥，别写"修改"这种废话）
git commit -m "实现HandEvaluator豹子牌型判定+单元测试"

# ⑥ 推送到自己的分支
git push
```

**提交信息规范**：一句话说清"做了什么 + 为什么"。
好例子：`实现牌型比较:豹子>同花顺>金花,含异花235特殊规则`
坏例子：`update`、`修改`、`111`、`asdf`

---

## 三、合并到 main（由成员 D 负责，每天下班前）

```bash
# D 在 main 分支上操作：
git checkout main
git pull                            # 确保 main 最新
git merge A                         # 合并 A 的改动（B/C/D 同理）
# 若有冲突：见下方"冲突处理"
git push                            # 推送到云端 main
git checkout D                      # 切回自己的工作分支继续干
```

> 其他成员**不要**直接 push 到 main，统一由 D 合并。

---

## 四、冲突处理（新手最怕，其实很简单）

**冲突 = 你和别人改了同一文件的同一处**，git 无法自动决定。

出现 `CONFLICT` 提示时：

```bash
# ① 看哪些文件冲突了
git status                # 标着 both modified 的就是

# ② 用编辑器打开冲突文件，找到这种标记：
# <<<<<<< HEAD
#   你的代码
# =======
#   别人的代码
# >>>>>>> A

# ③ 手动决定保留哪边（或都留），删掉 <<<<<<< ======= >>>>>>> 三行标记

# ④ 保存后：
git add -A
git commit -m "解决与A分支的冲突:保留双方牌型定义"
git push
```

**预防冲突的土办法**：每天开工先 `git pull`；各成员尽量只动自己模块的文件（A 动 core、B 动 ai、C 动 render/ui），不碰别人的文件。

---

## 五、常见问题速查

| 问题 | 原因 | 解决 |
|---|---|---|
| `Permission denied (publickey)` | SSH 公钥没加 | 重新执行"配置 SSH"步骤② |
| `failed to push some refs` | 别人先推了，你本地落后 | `git pull` → 再 `git push` |
| 改错文件想撤销 | — | 未提交：`git checkout -- 文件名`；已提交未推送：`git reset --soft HEAD~1` |
| 不小心提交到 main | 手滑 | `git reset --soft HEAD~1` 撤回，切回自己分支再提交 |
| push 提示要用户名密码 | 用的 HTTPS | 改用 SSH 方式（重新 clone 或改 remote） |
| `git pull` 后代码没了？ | 不会，pull 只会增加/更新 | `git log` 查看历史，`git checkout <commit>` 找回 |

---

## 六、团队约定（务必遵守）

1. **每天上班第一件事 `git pull`，下班前必须 `git push`**（代码留在自己电脑 = 白干）
2. **只 push 自己的分支**（A→A、B→B……），main 由 D 合并
3. 提交信息写清楚：`实现X功能/修复Y问题 + 一句话说明`
4. 尽量只改自己模块的文件，跨模块改动先在群里说一声
5. 改接口（头文件）必须群里报备，否则别人编译不过
6. 编译通过、测试通过再提交；提交前先本地 `cmake --build build` 验证
7. 遇到解决不了的问题 → 群里喊人，或让 AI 助手帮忙看

---

## 七、本项目的构建命令（Windows）

```bash
cmake -B build -G Ninja      # 首次会自动解压 third_party/SFML-2.6.1.zip
cmake --build build
build\SanDaoPoker.exe        # 或 VS 打开文件夹直接 F5
```

单元测试（可选）：

```bash
cmake -B build -G Ninja -DBUILD_TESTS=ON
ctest --test-dir build
```

---

*遇到没覆盖到的问题，把报错原文发群里，比截图更高效。*
