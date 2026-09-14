# -*- coding: utf-8 -*-
"""弹窗装饰边框素材生成器 v3(原创程序化绘制; 欧式结纹样版)

v3 相对 v2 的改动(按需求):
  - 线条细化: 珠链(每颗 3.5px 球, 有块状拼接感) -> 连续矢量细线 1~2px(主纹 2px / 内衬 1px)
    连续多段线 + 圆头线帽(PIL 的 joint="curve")天然解决"像素块不对接"
  - 纹样改为"结": 凯尔特结扣(Celtic knot) + 中国结式双钱结/编织结 + 涡卷结 + 四角结扣
  - 通体金色: 暗金描边(#8B6A24) + 金主线(#F2D68A) + 亮金高光(#FFF6D8)三层同色系

几何(9 宫格, 画布 128, 切片 48, 纹样带 4~40):
  - 上边: 双线绞索(twist knot) 2px + 上下内衬 1px   -> 拉伸到窗宽
  - 左边: 连续方结(weave knot) 2px 菱形网 + 内衬 1px -> 拉伸到窗高
  - 四角: 双钱结(两个相扣环) + 环上 8 字绞索(结扣感)
  - 结心宝石: 中心 1 颗小金珠点睛

输出: assets/ui/table/panel_frame.png (128x128, 切片 48)
依赖: Pillow
"""
from PIL import Image, ImageDraw
import math, os

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_ASSET = os.path.join(REPO, "assets", "ui", "table", "panel_frame.png")
OUT_PREVIEW = os.path.join(os.environ.get("TEMP", "."), "panel_frame_preview.png")

SS = 4                      # 超采样(细线抗锯齿关键)
S = 128
C = S * SS
SL = 48 * SS                # 切片
B0, B1 = 4, 40
band_c = (B0 + B1) / 2.0    # 22

# 金色三层(通体金色)
G_EDGE = (139, 106, 36)     # 暗金描边
G_MAIN = (242, 214, 138)    # 主金
G_HI = (255, 246, 216)      # 亮金高光
G_BAK = (74, 54, 16)        # 结下投影

img = Image.new("RGBA", (C, C), (0, 0, 0, 0))
dr = ImageDraw.Draw(img)


def P(pts):
    return [(x * SS, y * SS) for x, y in pts]


def line3(pts, w):
    """三层细线: 暗描边 -> 主金 -> 左上偏移亮金高光 (w = 主线宽, 单位: 成品像素)"""
    if len(pts) < 2:
        return
    we = max(1, int(round((w + 0.9) * SS)))
    wm = max(1, int(round(w * SS)))
    wh = max(1, int(round((w * 0.42) * SS)))
    off = 0.30 * SS
    dr.line(P(pts), fill=G_EDGE, width=we, joint="curve")
    for i in (0, -1):
        dr.line(P(pts), fill=G_EDGE, width=we, joint="curve")
    dr.line(P(pts), fill=G_MAIN, width=wm, joint="curve")
    hp = [(x * SS - off, y * SS - off) for x, y in pts]
    dr.line(hp, fill=G_HI, width=wh, joint="curve")


def dots(pts, r=0.9, col=G_HI):
    for x, y in pts:
        dr.ellipse([x * SS - r * SS, y * SS - r * SS, x * SS + r * SS, y * SS + r * SS], fill=col)


def twist_path(horiz, length, c_off, period=16.0, amp=3.4):
    """绞索/双线绞: 两股正弦(相位差 pi)缠绕, 返回 [strandA, strandB]
    周期 16px: 恰好整除切片宽 32px -> 平铺时无缝(拼接处相位=0)"""
    a, b = [], []
    n = max(24, int(length / 0.8))
    for i in range(n + 1):
        s = i / n * length
        o1 = amp * math.sin(2 * math.pi * s / period)
        o2 = amp * math.sin(2 * math.pi * s / period + math.pi)
        if horiz:
            a.append((s, c_off + o1)); b.append((s, c_off + o2))
        else:
            a.append((c_off + o1, s)); b.append((c_off + o2, s))
    return a, b


def weave_path(horiz, length, c_off, amp=5.0, step=8.0):
    """连续方结(编织结): 菱形折线, 返回单条折线"""
    pts = []
    up = True
    s = 0.0
    while s <= length + 0.01:
        o = -amp if up else amp
        pts.append((s, c_off + o) if horiz else (c_off + o, s))
        up = not up
        s += step
    return pts


def eight_path(cx, cy, r=5.6, turns=2.0, n=90):
    """8 字绞(结扣感): 双环交叠的连续曲线"""
    pts = []
    for i in range(n + 1):
        t = i / n * 2 * math.pi * turns / 2.0
        x = cx + r * math.sin(t) * math.cos(t) * 2.0
        y = cy + r * math.sin(t * 1.0) * 1.0
        pts.append((x, y))
    return pts


def cabochon(cx, cy, r=2.6):
    """结心小金珠点睛"""
    dr.ellipse([(cx - r) * SS, (cy - r) * SS, (cx + r) * SS, (cy + r) * SS], fill=G_EDGE)
    dr.ellipse([(cx - r * 0.78) * SS, (cy - r * 0.78) * SS, (cx + r * 0.78) * SS, (cy + r * 0.78) * SS], fill=G_MAIN)
    dr.ellipse([(cx - r * 0.30) * SS, (cy - r * 0.36) * SS, (cx + r * 0.16) * SS, (cy + r * 0.10) * SS], fill=G_HI)


# ---------------------------------------------------------------
# 1) 四条边: 主纹(2px) + 上下内衬(1px)
# ---------------------------------------------------------------
for side in range(4):
    horiz = (side in (0, 1))
    c_off = band_c if side in (0, 2) else S - band_c
    # 主纹: 上/下边 = 双线绞索; 左/右边 = 连续方结
    if side in (0, 1):
        a, b = twist_path(horiz, S, c_off)
        line3(a, 2.0)
        line3(b, 2.0)
    else:
        wk = weave_path(horiz, S, c_off)
        line3(wk, 2.0)
    # 内衬细线(1px): 纹样带上下各一条
    off1, off2 = band_c - 9.0, band_c + 9.0
    for o in (off1, off2):
        if horiz:
            line3([(0, o if side == 0 else S - o), (S, o if side == 0 else S - o)], 1.0)
        else:
            line3([(o if side == 2 else S - o, 0), (o if side == 2 else S - o, S)], 1.0)

# ---------------------------------------------------------------
# 2) 四角: 双钱结(相扣双环) + 8 字绞结扣
# ---------------------------------------------------------------
def corner_knot(cx, cy, sx, sy):
    # 双钱结: 两个相交圆环(略压扁), 环线 2px
    r = 8.6
    for k, (ox, oy) in enumerate(((0.0, 0.0), (sx * 6.2, sy * 6.2))):
        ring = []
        n = 72
        for i in range(n + 1):
            t = i / n * 2 * math.pi
            ring.append((cx + ox + r * math.cos(t) * 0.92,
                         cy + oy + r * math.sin(t) * 0.78))
        line3(ring, 2.0)
    # 结扣: 双环交点处的小 8 字绞
    p = eight_path(cx + sx * 3.1, cy + sy * 3.1, 5.4)
    line3(p, 1.6)
    # 环外沿绞索点缀(沿对角方向)
    tw = []
    n = 60
    for i in range(n + 1):
        t = i / n
        a = math.atan2(sy, sx) + (t - 0.5) * 1.15
        rr = 15.5
        tw.append((cx + rr * math.cos(a), cy + rr * math.sin(a)))
    line3(tw, 1.6)
    cabochon(cx + sx * 1.6, cy + sy * 1.6, 2.4)


corner_knot(B0 + 5.0, B0 + 5.0, 1, 1)
corner_knot(S - B0 - 5.0, B0 + 5.0, -1, 1)
corner_knot(B0 + 5.0, S - B0 - 5.0, 1, -1)
corner_knot(S - B0 - 5.0, S - B0 - 5.0, -1, -1)

final = img.resize((S, S), Image.LANCZOS)
os.makedirs(os.path.dirname(OUT_ASSET), exist_ok=True)
final.save(OUT_ASSET)
print("saved:", OUT_ASSET, final.size)


# ---------------------------------------------------------------
# 3) 预览(与 C++ 助手同一 9 宫格算法)
# ---------------------------------------------------------------
def nine_slice(panel_w, panel_h, band=48):
    canvas = Image.new("RGBA", (panel_w + band * 2, panel_h + band * 2), (0, 0, 0, 0))
    canvas.paste(Image.new("RGBA", (panel_w, panel_h), (30, 40, 70, 255)), (band, band))
    ImageDraw.Draw(canvas).rectangle([band, band, band + panel_w - 1, band + panel_h - 1],
                                     outline=(255, 215, 0, 255), width=3)
    cuts = [0, 48, S - 48, S]
    w_out, h_out = panel_w + band * 2, panel_h + band * 2
    xs = [0, 48, w_out - 48, w_out]
    ys = [0, 48, h_out - 48, h_out]
    for r in range(3):
        for c in range(3):
            piece = final.crop((cuts[c], cuts[r], cuts[c + 1], cuts[r + 1]))
            w = max(1, xs[c + 1] - xs[c]); h = max(1, ys[r + 1] - ys[r])
            if piece.size != (w, h):
                piece = piece.resize((w, h), Image.LANCZOS)
            canvas.alpha_composite(piece, (xs[c], ys[r]))
    return canvas


nine_slice(640, 360).save(OUT_PREVIEW)
print("saved preview:", OUT_PREVIEW)
