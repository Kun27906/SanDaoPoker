# -*- coding: utf-8 -*-
"""弹窗装饰边框素材生成器(原创程序化绘制)

用途: 重新生成 assets/ui/table/panel_frame.png (弹窗装饰边框素材)
      调整纹样宽度/配色后重跑本脚本即可更新素材; 游戏侧无需改动(9 宫格自适应)。

风格: 巴洛克/洛可可装饰语汇 —— 缠丝双股辫 + 细金丝线 + 内圈珍珠链
      + 四角涡卷叶饰 + 石榴红浆果点缀(全部原创程序化合成, 非图库素材)。

要点:
  - 画布 128x128, 9 宫格切片 48px(四角固定 1:1, 四边拉伸, 中心透明)
  - "凸起浮雕"工艺: 每条线按 暗描边 -> 中间金 -> 左上偏移高光 三遍绘制,
    再垫一层右下偏移投影; 光向统一左上
  - 三遍法很重要: 逐颗绘制会让后一颗的暗边盖住前一颗的中间色, 整体发黑

依赖: Pillow (pip install pillow)
用法: py tools/gen_panel_frame.py
"""
from PIL import Image, ImageDraw
import math, os, sys

# ---- 路径: <仓库根>/assets/ui/table/panel_frame.png ----
REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_ASSET = os.path.join(REPO, "assets", "ui", "table", "panel_frame.png")
OUT_PREVIEW = os.path.join(os.environ.get("TEMP", "."), "panel_frame_preview.png")

SS = 4                      # 超采样倍数
S = 128                     # 成品尺寸
C = S * SS
B0, B1 = 4, 40              # 纹样带范围(距边缘像素)
band_c = (B0 + B1) / 2.0

# 调色板(提亮版)
HI = (255, 246, 214)
LT = (250, 226, 168)
MID = (226, 186, 108)
MID2 = (186, 144, 70)
DK = (128, 92, 34)
EDGE = (58, 40, 12)
SHADOW = (8, 5, 1, 130)
GARNET = (150, 62, 54)

img = Image.new("RGBA", (C, C), (0, 0, 0, 0))
dr = ImageDraw.Draw(img)


def ell(cx, cy, r, color):
    x, y, rr = cx * SS, cy * SS, r * SS
    dr.ellipse([x - rr, y - rr, x + rr, y + rr], fill=color)


def chain3(pts, r_of, mid=MID, hi=HI, edge=EDGE):
    """三遍绘制珠链: 全部暗边 -> 全部中间色 -> 全部高光(左上偏移)"""
    for i, (x, y) in enumerate(pts):
        ell(x, y, r_of(i), edge)
    for i, (x, y) in enumerate(pts):
        ell(x, y, r_of(i) * 0.78, mid)
    for i, (x, y) in enumerate(pts):
        ell(x - 0.22, y - 0.30, r_of(i) * 0.34, hi)


def lone_ball(x, y, r, mid=MID, hi=HI, edge=EDGE):
    """孤立球体(珍珠/浆果)"""
    ell(x, y, r, edge)
    ell(x, y, r * 0.78, mid)
    ell(x - 0.18, y - 0.26, r * 0.34, hi)


def shadow_chain(pts, r_of):
    for i, (x, y) in enumerate(pts):
        ell(x + 0.85, y + 0.95, r_of(i) * 1.05, SHADOW)


def arc_pts(cx, cy, r0, r1, a0, a1, n=90, pw=1.25):
    out = []
    for i in range(n + 1):
        t = i / n
        a = a0 + (a1 - a0) * t
        r = r0 + (r1 - r0) * (t ** pw)
        out.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return out


# 1) 四条边投影带(凸起底)
for side in range(4):
    pts = []
    for i in range(241):
        t = i / 240.0
        if side == 0:
            pts.append((t * S, band_c))
        elif side == 1:
            pts.append((t * S, S - band_c))
        elif side == 2:
            pts.append((band_c, t * S))
        else:
            pts.append((S - band_c, t * S))
    shadow_chain(pts, lambda i: (B1 - B0) / 2.0 * 0.66)

# 2) 双股交织丝线辫(相位差 pi, 明暗交替)
PERIOD, AMP = 30.0, 5.2
for side in range(4):
    for k, phase in enumerate((0.0, math.pi)):
        pts = []
        for i in range(421):
            s = i / 420.0 * S
            off = AMP * math.sin(2 * math.pi * s / PERIOD + phase)
            if side == 0:
                pts.append((s, band_c + off))
            elif side == 1:
                pts.append((s, S - band_c + off))
            elif side == 2:
                pts.append((band_c + off, s))
            else:
                pts.append((S - band_c + off, s))
        if k == 0:
            chain3(pts, lambda i: 3.5, mid=MID, hi=HI)
        else:
            chain3(pts, lambda i: 3.3, mid=MID2, hi=LT)

# 3) 外侧细金丝线
FIL = 11.0
for side in range(4):
    s = 5.0
    while s <= S - 5.0:
        if side == 0:
            lone_ball(s, FIL, 1.5)
        elif side == 1:
            lone_ball(s, S - FIL, 1.5)
        elif side == 2:
            lone_ball(FIL, s, 1.5)
        else:
            lone_ball(S - FIL, s, 1.5)
        s += 6.0

# 4) 内圈珍珠链
PR = 45.0
for side in range(4):
    s = 7.0
    while s <= S - 7.0:
        if side == 0:
            lone_ball(s, PR, 2.1, mid=LT)
        elif side == 1:
            lone_ball(s, S - PR, 2.1, mid=LT)
        elif side == 2:
            lone_ball(PR, s, 2.1, mid=LT)
        else:
            lone_ball(S - PR, s, 2.1, mid=LT)
        s += 7.5


# 5) 四角涡卷叶饰 + 浆果
def corner_flourish(cx, cy, sx, sy, a0, a1):
    main = arc_pts(cx, cy, 2.5, 18.0, a0, a1, 100)
    shadow_chain(main, lambda i: 3.8 * (1.0 - 0.5 * i / 100.0))
    chain3(main, lambda i: 3.6 * (1.0 - 0.5 * i / 100.0))
    for frac, ln, sw in ((0.30, 21.0, 1.0), (1.00, 18.0, -1.0)):
        a = a0 + (a1 - a0) * frac
        x0, y0 = cx + 3.2 * math.cos(a), cy + 3.2 * math.sin(a)
        lp = []
        for i in range(46):
            t = i / 45.0
            aa = a + sw * 1.2 * t
            rr = ln * t
            lp.append((x0 + rr * math.cos(aa), y0 + rr * math.sin(aa)))
        shadow_chain(lp, lambda i: 3.4 * (1.0 - 0.62 * i / 45.0))
        chain3(lp, lambda i: 3.2 * (1.0 - 0.62 * i / 45.0), mid=LT, hi=HI)
    for j, rr in enumerate((3.8, 2.9, 2.2)):
        bx = cx + sx * (8.0 + j * 3.6 + rr)
        by = cy + sy * (8.0 + j * 2.4 + rr)
        lone_ball(bx, by, rr, mid=GARNET if j == 0 else MID, hi=HI,
                  edge=(60, 16, 12) if j == 0 else EDGE)


corner_flourish(B0 + 3.0,     B0 + 3.0,      1,  1,  math.pi * 0.15, math.pi * 0.95)
corner_flourish(S - B0 - 3.0, B0 + 3.0,     -1,  1,  math.pi * 0.85, math.pi * 1.85)
corner_flourish(B0 + 3.0,     S - B0 - 3.0,  1, -1, -math.pi * 0.15, -math.pi * 0.95)
corner_flourish(S - B0 - 3.0, S - B0 - 3.0, -1, -1, -math.pi * 0.85, -math.pi * 1.85)

final = img.resize((S, S), Image.LANCZOS)
os.makedirs(os.path.dirname(OUT_ASSET), exist_ok=True)
final.save(OUT_ASSET)
print("saved:", OUT_ASSET, final.size)


# 6) 预览(与 C++ 助手同一 9 宫格算法)
def nine_slice(panel_w, panel_h, band=48):
    canvas = Image.new("RGBA", (panel_w + band * 2, panel_h + band * 2), (0, 0, 0, 0))
    canvas.paste(Image.new("RGBA", (panel_w, panel_h), (30, 40, 70, 255)), (band, band))
    ImageDraw.Draw(canvas).rectangle(
        [band, band, band + panel_w - 1, band + panel_h - 1],
        outline=(255, 215, 0, 255), width=3)
    sl = 48
    cuts = [0, sl, S - sl, S]
    w_out, h_out = panel_w + band * 2, panel_h + band * 2
    xs = [0, sl, w_out - sl, w_out]
    ys = [0, sl, h_out - sl, h_out]
    for r in range(3):
        for c in range(3):
            piece = final.crop((cuts[c], cuts[r], cuts[c + 1], cuts[r + 1]))
            w = max(1, xs[c + 1] - xs[c])
            h = max(1, ys[r + 1] - ys[r])
            if piece.size != (w, h):
                piece = piece.resize((w, h), Image.LANCZOS)
            canvas.alpha_composite(piece, (xs[c], ys[r]))
    return canvas


nine_slice(640, 360).save(OUT_PREVIEW)
print("saved preview:", OUT_PREVIEW)
