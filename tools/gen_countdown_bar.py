# 组牌倒计时条贴图
# 输出: assets/ui/table/countdown_bar_bg.png 与 countdown_fill_{green,yellow,red}.png
import os
from PIL import Image, ImageDraw

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(BASE, "assets", "ui", "table")

W, H = 400, 24
FILLS = {"green": (70, 190, 90), "yellow": (235, 190, 50), "red": (220, 60, 50)}


def save(name, fill, outline):
    im = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    ImageDraw.Draw(im).rounded_rectangle([0, 0, W - 1, H - 1], H // 2,
                                         fill=fill, outline=outline, width=2)
    os.makedirs(OUT, exist_ok=True)
    im.save(os.path.join(OUT, name))
    print("生成:", name)


if __name__ == "__main__":
    save("countdown_bar_bg.png", (20, 24, 32, 160), (255, 255, 255, 90))
    for name, rgb in FILLS.items():
        save("countdown_fill_" + name + ".png", rgb, (255, 255, 255, 140))
