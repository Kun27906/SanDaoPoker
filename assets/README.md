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
cards/back/<color>.png     牌背: black|blue|red
cards/joker_black.png      大王（黑色 JOKER）
cards/joker_red.png        小王（红色 JOKER）
```

素材来源：[SVG-cards 4.0](https://github.com/htdebeer/SVG-cards)（LGPL-2.1，经典法式牌面，J/Q/K 带人物画像）。
程序生成的旧几何牌面已替换。后续想换画风可重新渲染（Edge headless 渲染 SVG，源文件在 `C:\Users\27906\SVG-cards`）。

## UI 素材

| 文件 | 说明 |
|------|------|
| `ui/backgrounds/table_bg.png` | 游戏桌面底图（2000×1200，绿呢赌桌） |
| `ui/buttons/btn_normal.png` | 按钮-常态（240×80，透明圆角） |
| `ui/buttons/btn_hover.png` | 按钮-悬停 |
| `ui/buttons/btn_pressed.png` | 按钮-按下 |
| `ui/buttons/btn_disabled.png` | 按钮-禁用 |
| `fonts/SourceHanSansSC-Regular.otf` | 思源黑体（开源可分发） |

生成脚本：`tools/gen_ui_assets.py`（底图+按钮，改风格重跑即可）

## 小素材（第4天）

| 文件 | 说明 |
|------|------|
| `ui/table/countdown_bar_bg.png` | 倒计时条底（400×24，深色圆角） |
| `ui/table/countdown_fill_green/yellow/red.png` | 倒计时填充（按剩余时间选色） |
| `ui/chips/chip_1/5/10/50/100.png` | 筹码 5 种面值（80×80） |
| `ui/table/mark_win.png` | 胜利标记（胜，绿） |
| `ui/table/mark_lose.png` | 失败标记（负，灰） |
| `ui/table/mark_draw.png` | 平局标记（平，黄） |
