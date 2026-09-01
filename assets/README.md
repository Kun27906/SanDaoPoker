# assets — 游戏资源目录

构建时由 CMake 自动复制到可执行文件所在目录（见根 `CMakeLists.txt`）。
空目录以 `.gitkeep` 占位，保证克隆后目录结构完整。

| 目录 | 用途 |
|------|------|
| `cards/` | 扑克牌贴图：55 张（4 花色 × 13 点数 + 3 种牌背），PNG 200×280 |
| `ui/` | 界面素材：`backgrounds/` 背景图、`buttons/` 按钮三态图、`chips/` 筹码、`icons/` 图标、`table/` 桌面 |
| `fonts/` | 字体文件（如思源黑体） |
| `sounds/` | 音效与背景音乐（发牌/翻牌/筹码/胜负/BGM） |

## 牌图命名约定（AssetManager 加载用）

```
cards/<suit>/<rank>.png    花色: spades|hearts|clubs|diamonds
                           点数: A 2 3 4 5 6 7 8 9 10 J Q K
cards/back/<color>.png     牌背: black|blue|red
```

生成脚本：`tools/gen_card_sheet.py`（改风格后重跑即可重新生成全部牌图）
