# 发牌牌堆贴图生成器
# 输出: assets/cards/back/deck_pile_{red,blue,black}.png
import os
from PIL import Image, ImageDraw

BACK_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                        "assets", "cards", "back")

LAYERS = 54
STEP = 9
PAPER_W = 7
PAD = 6
PAPER = (250, 248, 242, 255)
SEP = (70, 30, 30, 235)


def build(color):
    back = Image.open(os.path.join(BACK_DIR, color + ".png")).convert("RGBA")
    bw, bh = back.size
    canvas = Image.new("RGBA", (bw + STEP * (LAYERS - 1) + PAD * 2, bh + PAD * 2),
                       (0, 0, 0, 0))
    for i in range(LAYERS - 1, -1, -1):
        cell_w = bw + PAPER_W + 2
        cell = Image.new("RGBA", (cell_w, bh + 2), (0, 0, 0, 0))
        d = ImageDraw.Draw(cell)
        d.rectangle([0, 0, cell_w - 1, bh + 1], fill=PAPER)
        cell.paste(back, (0, 1), back)
        d.line([(cell_w - 2, 0), (cell_w - 2, bh + 1)], fill=SEP, width=2)
        d.line([(bw, 0), (cell_w - 1, 0)], fill=SEP, width=1)
        d.line([(bw, bh + 1), (cell_w - 1, bh + 1)], fill=SEP, width=1)
        k = i / max(1, LAYERS - 1)          # 越靠底层越暗
        cell = Image.alpha_composite(cell, Image.new("RGBA", cell.size,
                                                     (0, 0, 0, int(24 + 44 * k))))
        canvas.alpha_composite(cell, (PAD + STEP * i, PAD))
    return canvas


if __name__ == "__main__":
    for c in ("red", "blue", "black"):
        im = build(c)
        p = os.path.join(BACK_DIR, "deck_pile_" + c + ".png")
        im.save(p)
        print("生成:", os.path.basename(p), im.size)
