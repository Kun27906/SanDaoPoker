# 牌面与牌背贴图生成器
# 输出: assets/cards/<花色>/<点数>.png 52 张, assets/cards/back/{red,blue,black}.png
import os
from PIL import Image, ImageDraw
from gen_common import CARD_W, CARD_H, BG, BLACK, RED, font, rounded_card

OUT = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                   "assets", "cards")

SUITS = [("spades", "♠", BLACK), ("hearts", "♥", RED),
         ("clubs", "♣", BLACK), ("diamonds", "♦", RED)]
RANKS = ["A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"]


def draw_card(d, img, suit_sym, suit_color, rank):
    f_rank, f_suit = font(26, bold=True), font(20)
    d.text((14, 8), rank, font=f_rank, fill=suit_color)
    d.text((18, 38), suit_sym, font=f_suit, fill=suit_color)

    corner = Image.new("RGBA", (60, 60), (0, 0, 0, 0))
    cd = ImageDraw.Draw(corner)
    cd.text((2, 0), rank, font=f_rank, fill=suit_color)
    cd.text((6, 26), suit_sym, font=f_suit, fill=suit_color)
    rot = corner.rotate(180)
    img.paste(rot, (CARD_W - 58, CARD_H - 58), rot)

    f_mid = font(44)
    if rank in ("J", "Q", "K"):
        d.text((CARD_W // 2 - 22, CARD_H // 2 - 48), rank,
               font=font(56, bold=True), fill=suit_color)
        d.text((CARD_W // 2 - 12, CARD_H // 2 + 16), suit_sym, font=f_mid, fill=suit_color)
    elif rank == "A":
        d.text((CARD_W // 2 - 38, CARD_H // 2 - 52), "A",
               font=font(88, bold=True), fill=suit_color)
        d.text((CARD_W // 2 - 24, CARD_H // 2 + 40), suit_sym, font=f_mid, fill=suit_color)
    else:
        d.text((CARD_W // 2 - 38, CARD_H // 2 - 48), suit_sym,
               font=font(88), fill=suit_color)


def draw_back(d, rgb):
    d.rectangle([6, 6, CARD_W - 6, CARD_H - 6], fill=rgb)
    d.rectangle([10, 10, CARD_W - 10, CARD_H - 10], outline=(255, 255, 255), width=2)
    cx, cy = CARD_W // 2, CARD_H // 2
    d.polygon([(cx, cy - 34), (cx + 26, cy), (cx, cy + 34), (cx - 26, cy)],
              outline=(255, 255, 255), width=3)
    d.polygon([(cx, cy - 20), (cx + 15, cy), (cx, cy + 20), (cx - 15, cy)],
              fill=(255, 255, 255))


def save_flat(im, path, bg):
    flat = Image.new("RGB", im.size, bg)
    flat.paste(im, mask=im.split()[3])
    flat.save(path)
    print("生成:", os.path.relpath(path, OUT))


def main():
    for suit_dir, sym, color in SUITS:
        d = os.path.join(OUT, suit_dir)
        os.makedirs(d, exist_ok=True)
        for rank in RANKS:
            im = rounded_card(Image.new("RGBA", (CARD_W, CARD_H), BG))
            draw_card(ImageDraw.Draw(im), im, sym, color, rank)
            save_flat(im, os.path.join(d, rank + ".png"), BG)

    backs = [("black", (40, 42, 52)), ("blue", (36, 74, 148)), ("red", (178, 52, 58))]
    for name, rgb in backs:
        d = os.path.join(OUT, "back")
        os.makedirs(d, exist_ok=True)
        im = Image.new("RGBA", (CARD_W, CARD_H), (0, 0, 0, 0))
        draw_back(ImageDraw.Draw(im), rgb)
        save_flat(im, os.path.join(d, name + ".png"), (0, 0, 0))

    print("完成, 输出到", OUT)


if __name__ == "__main__":
    main()
