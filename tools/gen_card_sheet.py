# -*- coding: utf-8 -*-
"""
炸金花三道 - 55 张牌贴图生成器
用法: python tools/gen_card_sheet.py
输出: assets/cards/<花色>/<点数>.png (52张) + assets/cards/back/<颜色>.png (3张)
风格: 200x280 米色圆角扑克牌,标准"左上角+右下角点数/花色+中心大花色"布局
依赖: pip install pillow
"""
import os
from PIL import Image, ImageDraw, ImageFont

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(BASE, "assets", "cards")

CARD_W, CARD_H = 200, 280
CORNER = 12  # 圆角半径
BG = (241, 234, 218)        # 米色牌底
BORDER = (180, 168, 145)    # 浅棕边框
BLACK = (30, 30, 30)
RED = (192, 46, 46)

SUITS = [("spades", "♠", BLACK), ("hearts", "♥", RED),
         ("clubs", "♣", BLACK), ("diamonds", "♦", RED)]
RANKS = ["A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"]


def font(size, bold=False):
    """优先取系统常见字体,找不到就默认"""
    for name in (["arialbd.ttf", "arial.ttf"] if bold else ["arial.ttf"]):
        try:
            p = os.path.join(os.environ.get("WINDIR", "C:/Windows"), "Fonts", name)
            if os.path.exists(p):
                return ImageFont.truetype(p, size)
        except Exception:
            pass
    return ImageFont.load_default()


def draw_card(d, img, suit_sym, suit_color, rank):
    """画一张普通牌:角标(左上+右下旋转) + 中心大花色"""
    # 左上角:点数 + 花色
    f_rank = font(26, bold=True)
    f_suit = font(20)
    x, y = 14, 8
    d.text((x, y), rank, font=f_rank, fill=suit_color)
    d.text((x + 4, y + 30), suit_sym, font=f_suit, fill=suit_color)

    # 右下角:旋转180度复制一份
    corner = Image.new("RGBA", (60, 60), (0, 0, 0, 0))
    cd = ImageDraw.Draw(corner)
    cd.text((2, 0), rank, font=f_rank, fill=suit_color)
    cd.text((6, 26), suit_sym, font=f_suit, fill=suit_color)
    corner = corner.rotate(180)
    img.paste(corner, (CARD_W - 58, CARD_H - 58), corner)

    # 中心:大花色(点数牌)或字母+花色(JQK)
    f_mid = font(44)
    if rank in ("J", "Q", "K"):
        f_big = font(56, bold=True)
        d.text((CARD_W // 2 - 22, CARD_H // 2 - 48), rank,
               font=f_big, fill=suit_color)
        d.text((CARD_W // 2 - 12, CARD_H // 2 + 16), suit_sym,
               font=f_mid, fill=suit_color)
    elif rank == "A":
        f_big = font(88, bold=True)
        d.text((CARD_W // 2 - 38, CARD_H // 2 - 52), "A",
               font=f_big, fill=suit_color)
        d.text((CARD_W // 2 - 24, CARD_H // 2 + 40), suit_sym,
               font=f_mid, fill=suit_color)
    else:
        f_big = font(88)
        d.text((CARD_W // 2 - 38, CARD_H // 2 - 48), suit_sym,
               font=f_big, fill=suit_color)


def draw_back(d, color_name, rgb):
    """画牌背:纯色 + 白色细框 + 中心菱形"""
    d.rectangle([6, 6, CARD_W - 6, CARD_H - 6], fill=rgb)
    d.rectangle([10, 10, CARD_W - 10, CARD_H - 10], outline=(255, 255, 255), width=2)
    # 中心菱形花纹
    cx, cy = CARD_W // 2, CARD_H // 2
    d.polygon([(cx, cy - 34), (cx + 26, cy), (cx, cy + 34), (cx - 26, cy)],
              outline=(255, 255, 255), width=3)
    d.polygon([(cx, cy - 20), (cx + 15, cy), (cx, cy + 20), (cx - 15, cy)],
              fill=(255, 255, 255))
    return color_name


def main():
    img = None  # noqa: 占位,实际每张牌单独创建
    for suit_dir, sym, color in SUITS:
        d = os.path.join(OUT, suit_dir)
        os.makedirs(d, exist_ok=True)
        for rank in RANKS:
            im = Image.new("RGBA", (CARD_W, CARD_H), BG)
            dd = ImageDraw.Draw(im)
            # 圆角底 + 边框
            im = im.convert("RGBA")
            mask = Image.new("L", (CARD_W, CARD_H), 0)
            md = ImageDraw.Draw(mask)
            md.rounded_rectangle([0, 0, CARD_W - 1, CARD_H - 1], CORNER, fill=255)
            im.putalpha(mask)
            dd = ImageDraw.Draw(im)
            dd.rounded_rectangle([0, 0, CARD_W - 1, CARD_H - 1], CORNER,
                                 outline=BORDER, width=2)
            draw_card(dd, im, sym, color, rank)
            # 转 RGB(去掉透明,统一格式)
            flat = Image.new("RGB", im.size, BG)
            flat.paste(im, mask=im.split()[3])
            flat.save(os.path.join(d, f"{rank}.png"))
            print(f"生成 {suit_dir}/{rank}.png")

    # 牌背 x3
    backs = [("black", (40, 42, 52)), ("blue", (36, 74, 148)), ("red", (178, 52, 58))]
    for name, rgb in backs:
        d = os.path.join(OUT, "back")
        os.makedirs(d, exist_ok=True)
        im = Image.new("RGBA", (CARD_W, CARD_H), (0, 0, 0, 0))
        dd = ImageDraw.Draw(im)
        draw_back(dd, name, rgb)
        flat = Image.new("RGB", im.size, (0, 0, 0))
        flat.paste(im, mask=im.split()[3])
        flat.save(os.path.join(d, f"{name}.png"))
        print(f"生成 back/{name}.png")

    print(f"完成!共 {len(SUITS) * len(RANKS) + 3} 张牌图,输出到 {OUT}")


if __name__ == "__main__":
    main()
