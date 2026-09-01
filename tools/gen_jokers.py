# -*- coding: utf-8 -*-
"""
SanDaoPoker - 大小王贴图生成器
输出:
  assets/cards/Jokers/big.png    大王(红色系, "JOKER" + 皇冠)
  assets/cards/Jokers/small.png  小王(蓝灰系, "JOKER" + 皇冠)
风格: 200x280 米色圆角扑克牌, 与 gen_card_sheet.py 一致
依赖: pip install pillow
"""
import os
from PIL import Image, ImageDraw, ImageFont

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(BASE, "assets", "cards", "Jokers")

CARD_W, CARD_H = 200, 280
CORNER = 12
BG = (241, 234, 218)        # 米色牌底
BORDER = (180, 168, 145)    # 浅棕边框

def font(size, bold=False):
    for name in (["arialbd.ttf", "arial.ttf"] if bold else ["arial.ttf"]):
        try:
            p = os.path.join(os.environ.get("WINDIR", "C:/Windows"), "Fonts", name)
            if os.path.exists(p):
                return ImageFont.truetype(p, size)
        except Exception:
            pass
    return ImageFont.load_default()

def make_card(rank_text, suit_sym, suit_color, corner_color):
    """生成一张大小王牌面(居中大字 + 上下角标)"""
    im = Image.new("RGBA", (CARD_W, CARD_H), BG)
    mask = Image.new("L", (CARD_W, CARD_H), 0)
    md = ImageDraw.Draw(mask)
    md.rounded_rectangle([0, 0, CARD_W - 1, CARD_H - 1], CORNER, fill=255)
    dd = ImageDraw.Draw(im)
    dd.rounded_rectangle([0, 0, CARD_W - 1, CARD_H - 1], CORNER,
                         outline=BORDER, width=2)

    # 左上角标
    f_rank = font(24, bold=True)
    f_suit = font(18)
    dd.text((12, 8), rank_text, font=f_rank, fill=corner_color)
    dd.text((16, 36), suit_sym, font=f_suit, fill=corner_color)

    # 右下角标(旋转180度)
    corner = Image.new("RGBA", (60, 60), (0, 0, 0, 0))
    cd = ImageDraw.Draw(corner)
    cd.text((2, 0), rank_text, font=f_rank, fill=corner_color)
    cd.text((6, 26), suit_sym, font=f_suit, fill=corner_color)
    corner = corner.rotate(180)
    im.paste(corner, (CARD_W - 58, CARD_H - 58), corner)

    # 中心:大 JOKER 文字 + 皇冠符号
    f_big = font(52, bold=True)
    dd.text((CARD_W // 2 - 74, CARD_H // 2 - 66), "JOKER",
            font=f_big, fill=suit_color)
    f_mid = font(34)
    dd.text((CARD_W // 2 - 18, CARD_H // 2 + 6), suit_sym,
            font=f_mid, fill=suit_color)

    im = Image.composite(im, Image.new("RGBA", (CARD_W, CARD_H), (0, 0, 0, 0)), mask)
    return im

def main():
    os.makedirs(OUT, exist_ok=True)
    # 大王:红色
    big = make_card("B", "\u2655", (192, 46, 46), (120, 30, 30))
    big.save(os.path.join(OUT, "big.png"))
    print("生成:", os.path.join(OUT, "big.png"), big.size)
    # 小王:蓝灰
    small = make_card("S", "\u2655", (60, 90, 160), (40, 60, 110))
    small.save(os.path.join(OUT, "small.png"))
    print("生成:", os.path.join(OUT, "small.png"), small.size)

if __name__ == "__main__":
    main()
