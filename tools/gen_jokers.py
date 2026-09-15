# 大小王贴图生成器
# 输出: assets/cards/Jokers/{big,small}.png
import os
from PIL import Image, ImageDraw
from gen_common import CARD_W, CARD_H, CORNER, BG, BORDER, font

OUT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                   "assets", "cards", "Jokers")


def make_card(rank_text, suit_sym, suit_color, corner_color):
    im = Image.new("RGBA", (CARD_W, CARD_H), BG)
    ImageDraw.Draw(im).rounded_rectangle([0, 0, CARD_W - 1, CARD_H - 1], CORNER,
                                         outline=BORDER, width=2)
    dd = ImageDraw.Draw(im)

    f_rank, f_suit = font(24, bold=True), font(18)
    dd.text((12, 8), rank_text, font=f_rank, fill=corner_color)
    dd.text((16, 36), suit_sym, font=f_suit, fill=corner_color)

    corner = Image.new("RGBA", (60, 60), (0, 0, 0, 0))
    cd = ImageDraw.Draw(corner)
    cd.text((2, 0), rank_text, font=f_rank, fill=corner_color)
    cd.text((6, 26), suit_sym, font=f_suit, fill=corner_color)
    rot = corner.rotate(180)
    im.paste(rot, (CARD_W - 58, CARD_H - 58), rot)

    dd.text((CARD_W // 2 - 74, CARD_H // 2 - 66), "JOKER",
            font=font(52, bold=True), fill=suit_color)
    dd.text((CARD_W // 2 - 18, CARD_H // 2 + 6), suit_sym,
            font=font(34), fill=suit_color)

    mask = Image.new("L", (CARD_W, CARD_H), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, CARD_W - 1, CARD_H - 1], CORNER, fill=255)
    return Image.composite(im, Image.new("RGBA", (CARD_W, CARD_H), (0, 0, 0, 0)), mask)


def main():
    os.makedirs(OUT, exist_ok=True)
    for name, text, sym, color, corner in (
            ("big", "B", "\u2655", (192, 46, 46), (120, 30, 30)),
            ("small", "S", "\u2655", (60, 90, 160), (40, 60, 110))):
        p = os.path.join(OUT, name + ".png")
        make_card(text, sym, color, corner).save(p)
        print("生成:", p)


if __name__ == "__main__":
    main()
