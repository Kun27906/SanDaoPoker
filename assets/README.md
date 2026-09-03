# assets — 游戏资源目录

构建时由 CMake 自动复制到可执行文件所在目录（见根 `CMakeLists.txt`）。
空目录以 `.gitkeep` 占位，保证克隆后目录结构完整。

| 目录 | 用途 |
|------|------|
| `cards/` | 扑克牌贴图：57 张（4 花色 × 13 点数 + 3 种牌背 + 大小王），PNG 高清 338×488 |
| `ui/` | 界面素材：`backgrounds/` 背景图、`buttons/` 按钮三态图、`chips/` 筹码、`icons/` 图标、`table/` 桌面 |
| `fonts/` | 字体文件（思源黑体 Source Han Sans SC） |
| `sounds/` | 音效与背景音乐（发牌/翻牌/筹码/胜负/BGM） |

## 牌图命名约定（AssetManager 加载用）

```
cards/<suit>/<rank>.png    花色: spades|hearts|clubs|diamonds
                           点数: A 2 3 4 5 6 7 8 9 10 J Q K
cards/back/<color>.png     牌背: red|blue|black
cards/Jokers/small.png     小王（黑色 JOKER，万能黑花色）
cards/Jokers/big.png       大王（红色 JOKER，万能红花色）
```

素材来源：[SVG-cards 4.0](https://github.com/htdebeer/SVG-cards)（LGPL-2.1，经典法式牌面，J/Q/K 带人物画像）。
程序生成的旧几何牌面已替换。后续想换画风可重新渲染（Edge headless 渲染 SVG，源文件在 `C:\Users\27906\SVG-cards`）。

## UI 素材

| 文件 | 说明 |
|------|------|
| `ui/backgrounds/table_bg.png` | 游戏桌面背景（1920×1080，绿呢桌面） |
| `ui/backgrounds/menu.jpg` | 主菜单背景（1920×1080，场景画） |
| `ui/buttons/btn_normal.png` | 按钮-常态（240×80，亮金渐变） |
| `ui/buttons/btn_hover.png` | 按钮-悬停 |
| `ui/buttons/btn_pressed.png` | 按钮-按下 |
| `ui/buttons/btn_disabled.png` | 按钮-禁用 |
| `ui/chips/chip_1/5/10/25/50/100.png` | 筹码 6 种面值（416×416，3D 渲染高清，CC0） |
| `ui/icons/mark_win.png` | 胜利标记（绿底奖杯） |
| `ui/icons/mark_lose.png` | 失败标记（灰底叉） |
| `ui/icons/mark_draw.png` | 平局标记（黄底横杠） |
| `ui/table/countdown_bar_bg.png` | 倒计时条底 |
| `ui/table/countdown_fill_*.png` | 倒计时填充（绿/黄/红） |
| `fonts/SourceHanSansSC-Regular.otf` | 思源黑体（开源可分发） |

## 音效（assets/sounds/，CC0，Kenney casino-audio / interface-sounds）

| 文件 | 用途 | 播放时机 |
|------|------|----------|
| `deal.ogg` | 发牌 | 进入组牌场景 |
| `flip.ogg` | 翻牌 | 进入比牌场景 |
| `chip.ogg` | 筹码 | 进入结算场景 |
| `win.ogg` / `lose.ogg` | 胜负 | 结算判定（后续接入） |
| `shuffle.ogg` | 洗牌 | 备用 |
| `bet.ogg` | 下注 | 备用 |
| `click.ogg` | 按钮点击 | 所有按钮通用 |

接入：`SoundManager`（单例）加载与播放，`Button` 点击、`SceneManager` 场景切换已自动触发。

素材来源：poker_pack（CC0，筹码/桌），Kenney UI Pack（CC0，按钮），Kenney Game Icons（CC0，标记图标），[SVG-cards](https://github.com/htdebeer/SVG-cards)（LGPL-2.1，牌面）。
