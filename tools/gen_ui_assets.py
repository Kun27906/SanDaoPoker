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


# ---------------- 3. 倒计时条(底条 + 三色填充) ----------------
def gen_countdown():
    W, H = 400, 24
    # 底条:深色半透明圆角
    im = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([0, 0, W - 1, H - 1], H // 2, fill=(20, 24, 32, 160),
                        outline=(255, 255, 255, 90), width=2)
    im.save(os.path.join(UI, "table", "countdown_bar_bg.png"))
    print("生成: countdown_bar_bg.png")

    # 三色填充条(绿→黄→红,按剩余时间选)
    fills = {"green": (70, 190, 90), "yellow": (235, 190, 50),
             "red": (220, 60, 50)}
    for name, rgb in fills.items():
        im = Image.new("RGBA", (W, H), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.rounded_rectangle([0, 0, W - 1, H - 1], H // 2, fill=rgb)
        d.rounded_rectangle([0, 0, W - 1, H - 1], H // 2,
                            outline=(255, 255, 255, 140), width=2)
        im.save(os.path.join(UI, "table", f"countdown_fill_{name}.png"))
        print(f"生成: countdown_fill_{name}.png")


# ---------------- 4. 筹码(5 种面值) ----------------
def gen_chips():
    font_path = os.path.join(BASE, "assets", "fonts", "SourceHanSansSC-Regular.otf")
    size = 80
    chips = [
        ("chip_1",   (120, 130, 140), "1"),
        ("chip_5",   (200, 70, 70),   "5"),
        ("chip_10",  (60, 110, 200),  "10"),
        ("chip_50",  (50, 160, 90),   "50"),
        ("chip_100", (210, 170, 60),  "100"),
    ]
    for name, rgb, val in chips:
        im = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        r = size // 2
        # 外环(面值色)+ 白内环 + 中心色
        d.ellipse([2, 2, size - 2, size - 2], fill=rgb)
        d.ellipse([12, 12, size - 12, size - 12], fill=(255, 255, 255))
        d.ellipse([20, 20, size - 20, size - 20], fill=rgb)
        # 外环虚线感:一圈小方块(简化:画一圈弧线装饰)
        d.arc([6, 6, size - 6, size - 6], 0, 360, fill=(255, 255, 255, 200), width=3)
        # 面值文字
        try:
            f = ImageFont.truetype(font_path, 30)
        except Exception:
            f = ImageFont.load_default()
        bbox = d.textbbox((0, 0), val, font=f)
        tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
        d.text(((size - tw) // 2, (size - th) // 2 - bbox[1]), val,
               font=f, fill=(255, 255, 255))
        im.save(os.path.join(UI, "chips", name + ".png"))
        print(f"生成: chips/{name}.png ({val})")


# ---------------- 5. 胜负平标记 ----------------
def gen_marks():
    font_path = os.path.join(BASE, "assets", "fonts", "SourceHanSansSC-Regular.otf")
    size = 120
    marks = [
        ("mark_win",   (52, 168, 88),  "胜",  (255, 255, 255)),
        ("mark_lose",  (150, 155, 160), "负", (255, 255, 255)),
        ("mark_draw",  (225, 175, 50), "平",  (255, 255, 255)),
    ]
    for name, rgb, ch, txt_col in marks:
        im = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        d.ellipse([4, 4, size - 4, size - 4], fill=rgb)
        d.ellipse([4, 4, size - 4, size - 4], outline=(255, 255, 255), width=4)
        try:
            f = ImageFont.truetype(font_path, 56)
        except Exception:
            f = ImageFont.load_default()
        bbox = d.textbbox((0, 0), ch, font=f)
        tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
        d.text(((size - tw) // 2, (size - th) // 2 - bbox[1]), ch,
               font=f, fill=txt_col)
        im.save(os.path.join(UI, "table", name + ".png"))
        print(f"生成: table/{name}.png ({ch})")


if __name__ == "__main__":
    gen_background()
    gen_buttons()
    gen_countdown()
    gen_chips()
    gen_marks()
    print("UI 素材生成完毕")
