# -*- coding: utf-8 -*-
"""
牌堆贴图生成器(发牌环节用)
- 每层牌 = 原始卡片尺寸 200x280(与 assets/cards/back/*.png 一致,不缩放)
- 54 层(一副牌 52+2王),逐层向右错开,露出清晰牌边(纸白边+深色分隔线)
- 纯 2D,无透视/无投影
输出: assets/ui/table/deck_pile_{red,blue,black}.png
"""
import os
from PIL import Image, ImageDraw

REPO = r"C:\Users\27906\SanDaoPoker"
BACK_DIR = os.path.join(REPO, "assets", "cards", "back")
OUT_DIR = os.path.join(REPO, "assets", "ui", "table")
os.makedirs(OUT_DIR, exist_ok=True)

LAYERS = 54          # 一副牌
STEP = 9             # 每层向右错开(px)
PAPER_W = 7          # 露出的纸边宽度(px)
PAPER = (250, 248, 242, 255)   # 纸边白
SEP = (70, 30, 30, 235)        # 层间分隔线

def build(color, scale=1):
    back = Image.open(os.path.join(BACK_DIR, color + ".png")).convert("RGBA")
    if scale != 1:
        back = back.resize((back.width * scale, back.height * scale), Image.LANCZOS)
    bw, bh = back.size            # 200 x 280
    pad = 6
    W = bw + STEP * (LAYERS - 1) + pad * 2
    H = bh + pad * 2
    canvas = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    for i in range(LAYERS - 1, -1, -1):        # 底层(最右) -> 顶层(最左)
        cell_w = bw + PAPER_W + 2
        cell = Image.new("RGBA", (cell_w, bh + 2), (0, 0, 0, 0))
        d = ImageDraw.Draw(cell)
        d.rectangle([0, 0, cell_w - 1, bh + 1], fill=PAPER)   # 纸边底
        cell.paste(back, (0, 1), back)                        # 牌背
        d.line([(cell_w - 2, 0), (cell_w - 2, bh + 1)], fill=SEP, width=2)  # 层界线
        d.line([(bw, 0), (cell_w - 1, 0)], fill=SEP, width=1)
        d.line([(bw, bh + 1), (cell_w - 1, bh + 1)], fill=SEP, width=1)
        # 越靠底层越暗(厚度感)
        k = i / max(1, LAYERS - 1)
        ov = Image.new("RGBA", cell.size, (0, 0, 0, int(24 + 44 * k)))
        cell = Image.alpha_composite(cell, ov)
        canvas.alpha_composite(cell, (pad + STEP * i, pad))
    return canvas

for c in ("red", "blue", "black"):
    im = build(c, scale=1)
    p = os.path.join(OUT_DIR, "deck_pile_" + c + ".png")
    im.save(p)
    print(c, im.size, "->", os.path.basename(p))

# 预览(三色 + 与原卡片对比)
ims = [Image.open(os.path.join(OUT_DIR, "deck_pile_" + c + ".png")) for c in ("red", "blue", "black")]
card = Image.open(os.path.join(BACK_DIR, "red.png")).convert("RGBA")
H = max(i.height for i in ims) + 100
W = sum(i.width for i in ims) + card.width + 60 * 5
pv = Image.new("RGBA", (W, H), (45, 50, 60, 255))
x = 60
for im in ims:
    pv.alpha_composite(im, (x, 50)); x += im.width + 60
pv.alpha_composite(card, (x, 50))
pv.convert("RGB").save(os.path.join(REPO, "deck_pile_final_preview.jpg"), quality=92)
print("预览: deck_pile_final_preview.jpg (红/蓝/黑 牌堆 + 最右=原卡片200x280对比)")