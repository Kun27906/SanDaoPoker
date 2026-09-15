# 贴图生成公共定义
import os
from PIL import Image, ImageDraw, ImageFont

CARD_W, CARD_H = 200, 280
CORNER = 12
BG = (241, 234, 218)
BORDER = (180, 168, 145)
BLACK = (30, 30, 30)
RED = (192, 46, 46)


def font(size, bold=False):
    win_dir = os.environ.get("WINDIR")
    if win_dir:
        for name in (["arialbd.ttf", "arial.ttf"] if bold else ["arial.ttf"]):
            p = os.path.join(win_dir, "Fonts", name)
            if os.path.exists(p):
                return ImageFont.truetype(p, size)
    return ImageFont.load_default()


def rounded_card(img):
    w, h = img.size
    mask = Image.new("L", (w, h), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, w - 1, h - 1], CORNER, fill=255)
    img.putalpha(mask)
    ImageDraw.Draw(img).rounded_rectangle([0, 0, w - 1, h - 1], CORNER,
                                          outline=BORDER, width=2)
    return img
