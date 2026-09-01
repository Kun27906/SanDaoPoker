# -*- coding: utf-8 -*-
"""
第三天任务:UI 素材生成器(底图 + 按钮三态图)
输出:
  assets/ui/backgrounds/table_bg.png   游戏桌面背景(绿呢赌桌风格)
  assets/ui/buttons/btn_normal.png    按钮-常态
  assets/ui/buttons/btn_hover.png     按钮-悬停
  assets/ui/buttons/btn_pressed.png   按钮-按下
  assets/ui/buttons/btn_disabled.png  按钮-禁用(额外)
依赖: pip install pillow
"""
import os
from PIL import Image, ImageDraw, ImageFont

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
UI = os.path.join(BASE, "assets", "ui")

# ---------------- 1. 桌面底图(2000x1200 绿呢桌) ----------------
def gen_background():
    W, H = 2000, 1200
    im = Image.new("RGB", (W, H), (18, 92, 54))     # 深绿桌布
    d = ImageDraw.Draw(im)

    # 桌布纹理:细网格线
    for x in range(0, W, 40):
        d.line([(x, 0), (x, H)], fill=(24, 100, 60), width=1)
    for y in range(0, H, 40):
        d.line([(0, y), (W, y)], fill=(24, 100, 60), width=1)

    # 中央圆形赌桌(浅绿)
    cx, cy, r = W // 2, H // 2, 420
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(30, 120, 72))
    d.ellipse([cx - r + 14, cy - r + 14, cx + r - 14, cy + r - 14],
              outline=(58, 150, 96), width=6)
    # 桌心小圆(放筹码用)
    d.ellipse([cx - 90, cy - 90, cx + 90, cy + 90], fill=(26, 106, 64))
    d.ellipse([cx - 90, cy - 90, cx + 90, cy + 90],
              outline=(58, 150, 96), width=4)

    # 四角装饰花纹(简化:圆点)
    for px, py in [(150, 150), (W - 150, 150), (150, H - 150), (W - 150, H - 150)]:
        d.ellipse([px - 40, py - 40, px + 40, py + 40], fill=(22, 100, 60))
        d.ellipse([px - 40, py - 40, px + 40, py + 40],
                  outline=(58, 150, 96), width=4)

    out = os.path.join(UI, "backgrounds", "table_bg.png")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    im.save(out)
    print("生成:", out, im.size)


# ---------------- 2. 按钮三态图(无文字底图 240x80) ----------------
def gen_buttons():
    W, H = 240, 80
    states = {
        "btn_normal":   ((64, 120, 200), (90, 150, 230)),   # 蓝
        "btn_hover":    ((90, 160, 240), (120, 190, 255)),  # 亮蓝
        "btn_pressed":  ((40, 85, 150),  (55, 105, 175)),   # 深蓝
        "btn_disabled": ((140, 145, 152), (160, 165, 172)), # 灰
    }
    for name, (top, bottom) in states.items():
        im = Image.new("RGB", (W, H), bottom)
        d = ImageDraw.Draw(im)
        # 垂直渐变(上浅下深)
        for y in range(H):
            t = y / H
            col = tuple(int(top[i] * (1 - t) + bottom[i] * t) for i in range(3))
            d.line([(0, y), (W, y)], fill=col)
        # 圆角 + 边框 + 高光
        mask = Image.new("L", (W, H), 0)
        md = ImageDraw.Draw(mask)
        md.rounded_rectangle([0, 0, W - 1, H - 1], 16, fill=255)
        im.putalpha(mask)
        d = ImageDraw.Draw(im)
        d.rounded_rectangle([0, 0, W - 1, H - 1], 16, outline=(255, 255, 255), width=3)
        d.rounded_rectangle([6, 6, W - 7, H - 7], 12, outline=(255, 255, 255), width=1)
        # 上缘高光
        d.rounded_rectangle([10, 6, W - 10, 18], 8, fill=(255, 255, 255))

        out = os.path.join(UI, "buttons", name + ".png")
        os.makedirs(os.path.dirname(out), exist_ok=True)
        im.save(out)
        print("生成:", out, im.size)


if __name__ == "__main__":
    gen_background()
    gen_buttons()
    print("UI 素材生成完毕")
