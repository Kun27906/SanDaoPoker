# assets — 游戏资源目录

构建时由 CMake 自动同步到可执行文件所在目录（见 `cmake/sync_assets.cmake`；新增/修改/删除素材后直接 `cmake --build build` 即可）。

| 目录 | 用途 |
|------|------|
| `cards/` | 扑克牌贴图：57 张（4 花色 × 13 点数 + 3 种牌背 + 大小王），PNG 高清 338×488 |
| `cards/back/deck_pile_*` | 牌堆堆叠图（红/蓝/黑，689×292，54 层右侧露边）——发牌动画用 |
| `ui/` | 界面素材：`backgrounds/` 背景图、`buttons/` 按钮三态图、`chips/` 筹码、`icons/` 图标、`table/` 桌面小贴图、`avatars/` 头像 |
| `ai/` | AI 胜率表 `winrate.bin`（24804 种三张牌组合；离线生成，见 `tools/gen_winrate_table`） |
| `fonts/` | 字体文件（思源黑体 Source Han Sans SC） |
| `sounds/` | 音效与背景音乐 |

## 牌图命名约定（AssetManager 加载用）

```
cards/<suit>/<rank>.png    花色: spades|hearts|clubs|diamonds
                           点数: A 2 3 4 5 6 7 8 9 10 J Q K
cards/back/<color>.png     牌背: red|blue|black
cards/Jokers/small.png     小王（黑色 JOKER，万能黑花色）
cards/Jokers/big.png       大王（红色 JOKER，万能红花色）
ui/avatars/<任意名>.png    头像图（100×100 方形；加载时自动按内切圆做圆形裁剪，
                           文件名排序后按序分配给各玩家；无需手动裁圆）
ui/chips/chip_<区间>.png   筹码图标（文件名即档位区间，按余额自动选图）：
                           chip_-1000=<1000 | chip_1000-2000 | chip_2000-5000
                           chip_5000-10000 | chip_10000-15000 | chip_15000-=≥15000
ui/icons/<名>.png          界面图标（文件名即查询名，代码用 icon("<名>") 取）
ui/table/*.png             桌面小贴图（文件名即查询名，代码用 tableTexture("<名>") 取）
```

## UI 素材

| 文件 | 说明 |
|------|------|
| `ui/backgrounds/table_bg.png` | 游戏桌面背景（1920×1080，绿呢桌面） |
| `ui/backgrounds/menu.jpg` | 主菜单背景（1920×1080，场景画） |
| `ui/buttons/btn_normal.png` | 按钮-常态（240×80，亮金渐变） |
| `ui/buttons/btn_hover.png` | 按钮-悬停 |
| `ui/buttons/btn_pressed.png` | 按钮-按下 |
| `ui/buttons/btn_disabled.png` | 按钮-禁用 |
| `ui/chips/chip_*.png` | 筹码 6 档区间（416×416，3D 渲染高清，CC0） |
| `ui/icons/*.png` | 界面图标 100×100：menuList / musicOn / musicOff / soundSetting / soundOff / wrench / home / close / slider |
| `ui/table/countdown_bar_bg.png` | 倒计时条底槽（400×24）——组牌限时条 |
| `ui/table/countdown_fill_green/yellow/red.png` | 倒计时填充（绿 >2/3，黄 1/3~2/3，红 <1/3） |
| `fonts/SourceHanSansSC-Regular.otf` | 思源黑体（开源可分发） |

## 音效（assets/sounds/，CC0）

| 文件 | 用途 | 播放时机 |
|------|------|----------|
| `deal.ogg` | 发牌 | 发牌动画(逐张) |
| `flip.ogg` | 翻牌 | 比牌翻牌 / 发牌后手牌统一翻转 |
| `chip.ogg` | 筹码 | 点击筹码条左端筹码图标 |
| `win.mp3` | 胜利 | 整场结束且总盈利 |
| `lose.ogg` | 失败 | 整场结束且总亏损 |
| `bet.ogg` | 下注 | 每局开始（上一界面, 配合筹码数字滚动扣减） |
| `coins.wav` | 金币结算 | 每局结算 + 整场结束点击[返回大厅]（独立通道；配合筹码数字跳动） |
| `error.ogg` | 操作不可用 | 发牌/组牌/比牌界面点击 wrench（禁用开发者模式提示） |
| `count_down_clock.wav` | 倒计时时钟 | 组牌倒计时进入红色区（≤15%）循环播放（**二倍速**）, 交牌/超时停止 |
| `click.ogg` | 按钮点击 | 所有按钮/图标按钮通用 |
| `bgm_menu.mp3` / `bgm_game.mp3` | 背景音乐 | 菜单组 / 对局组循环（music 键开关） |

接入：`SoundManager`（单例）加载与播放；短音效有**主/副两条通道**（coins 走副通道，可与 win/lose 叠加）。
（注：洗牌音效 `shuffle.ogg` 已按设计决定移除——舍弃洗牌动画。）

素材来源：poker_pack（CC0，筹码/桌），Kenney UI Pack（CC0，按钮），Kenney casino-audio / interface-sounds（CC0，音效），[SVG-cards](https://github.com/htdebeer/SVG-cards)（LGPL-2.1，牌面）。

## 牌堆素材(发牌环节)

| 文件 | 说明 |
|------|------|
| `cards/back/deck_pile_red.png` | 红背牌堆(54层,右侧逐层露边,每层牌=200×280 原尺寸,689×292) |
| `cards/back/deck_pile_blue.png` | 蓝背牌堆(同上) |
| `cards/back/deck_pile_black.png` | 黑背牌堆(同上) |

生成脚本：`tools/gen_deck_pile.py`（改层数/错开量/纸边宽度后重跑即可）
用途：发牌环节展示牌堆；动画按"剩余张数"从右侧裁切。
