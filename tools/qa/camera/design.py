# Lane A214: a static model of the boards stage (no springs) to pick its numbers before the C++: sweep positions near the boards x facings.
import math, sys
from geom import *
L, W, R = [float(v) for v in (sys.argv[1:4] if len(sys.argv) > 3 else (90, 45, 3.75))]
D, H0, OFF, LA, MARGIN = 6.0, 3.5, 0.7, 5.0, 0.8
TANV = math.tan(math.radians(35)) / (16 / 9)
def ss(t): t = max(0.0, min(1.0, t)); return t * t * (3 - 2 * t)
def nearest_inside(x, y, m):
    hx, hy = L / 2 - m, W / 2 - m
    r = max(0.0, min(R - m, hx, hy))
    qx, qy = max(-hx, min(hx, x)), max(-hy, min(hy, y))
    if r > 0 and abs(qx) > hx - r and abs(qy) > hy - r:
        cx, cy = math.copysign(hx - r, qx), math.copysign(hy - r, qy)
        dx, dy = qx - cx, qy - cy; l = math.hypot(dx, dy)
        if l > r: qx, qy = cx + dx * r / l, cy + dy * r / l
    return qx, qy
def ray(x, y, a):
    # distance along a from (x,y) to the inner line
    lo, hi = 0.0, 200.0
    if not inside(x, y, L, W, R): return 0.0
    for _ in range(30):
        mid = (lo + hi) / 2
        if inside(x + math.cos(a) * mid, y + math.sin(a) * mid, L, W, R): lo = mid
        else: hi = mid
    return lo
def rig(px, py, yaw):
    a = math.radians(yaw); f = (math.cos(a), math.sin(a)); r = (-f[1], f[0])
    return (px - D * f[0] + OFF * r[0], py - D * f[1] + OFF * r[1], H0), -math.degrees(math.atan(H0 / (D + LA)))
def walls(px, py):
    return [(L / 2 - px, (1, 0)), (L / 2 + px, (-1, 0)), (W / 2 - py, (0, 1)), (W / 2 + py, (0, -1))]
def mine(px, py, yaw, P):
    a = math.radians(yaw); f = (math.cos(a), math.sin(a)); r = (-f[1], f[0])
    near, full = P['near'], P['full']
    ux = uy = 0.0; dmin = 1e9
    for d, n in walls(px, py):
        w = ss((near - d) / (near - full)); ux -= w * n[0]; uy -= w * n[1]; dmin = min(dmin, d)
    wgt = ss((P['gate'] - dmin) / (P['gate'] - P['gfull']))
    s = P['slide'] * (ux * r[0] + uy * r[1])
    fx, fy = px - D * f[0] + (OFF + s) * r[0], py - D * f[1] + (OFF + s) * r[1]
    fx, fy = nearest_inside(fx, fy, MARGIN)
    rho = (px - fx) * f[0] + (py - fy) * f[1]
    h = H0
    if P['lower']:
        h = max(P['hmin'], min(H0, 1.0 + max(rho, 0.0) * math.tan(math.radians(P['cap'] + 17.0))))
    c = ray(fx, fy, a)
    pitch_rig = math.degrees(math.atan(h / max(rho + LA, 1e-3)))
    pitch_ice = math.degrees(math.atan(h / max(c, 1e-3))) + math.degrees(math.atan(P['yt'] * TANV))
    want = max(pitch_rig, pitch_ice)
    rc = math.hypot(px - fx, py - fy)
    ac = math.degrees(math.atan2(h - 1.0, max(rho, 1e-3)))
    e = math.degrees(math.atan(P['keep'] * TANV))
    want = max(ac - e, min(ac + e, want))
    want = min(want, P['cap'])
    rx, rp = rig(px, py, yaw)
    # blend by the gate
    cam = (rx[0] + wgt * (fx - rx[0]), rx[1] + wgt * (fy - rx[1]), rx[2] + wgt * (h - rx[2]))
    pd = -rp + wgt * (want + rp)
    return cam, -pd
def measure(cam, yaw, pitch, px, py, vyaw=None):
    ice = ice_share(cam, yaw, pitch, L, W, R, cols=32, rows=18)
    ch = frac(cam, yaw, pitch, (px, py, 1.0))
    on = abs(ch[0]) <= 1 and abs(ch[1]) <= 1
    va = math.radians(yaw if vyaw is None else vyaw)
    ax, ay = px + 3 * math.cos(va), py + 3 * math.sin(va)
    ah = frac(cam, yaw, pitch, (ax, ay, 0.0)); aon = abs(ah[0]) <= 1 and abs(ah[1]) <= 1
    inb = inside(cam[0], cam[1], L, W, R, 0.5)
    return ice, -pitch, on, aon, inb
def sweep(fn):
    rows = []
    # positions along the boards' line, 0.6..8 m in, every 2 m of perimeter; facings every 15 degrees
    per = []
    for d in (0.6, 1.0, 2.0, 3.0, 5.0, 7.0):
        hx, hy = L / 2 - d, W / 2 - d
        n = int(2 * (2 * hx + 2 * hy) / 3)
        for k in range(n):
            t = k / n * (4 * hx + 4 * hy)
            if t < 2 * hx: p = (-hx + t, hy)
            elif t < 2 * hx + 2 * hy: p = (hx, hy - (t - 2 * hx))
            elif t < 4 * hx + 2 * hy: p = (hx - (t - 2 * hx - 2 * hy), -hy)
            else: p = (-hx, -hy + (t - 4 * hx - 2 * hy))
            p = nearest_inside(p[0], p[1], d)
            per.append((d, p))
    for d, (px, py) in per:
        for yaw in range(-180, 180, 15):
            cam, pitch = fn(px, py, yaw)
            rows.append((d,) + measure(cam, yaw, pitch, px, py))
    n = len(rows)
    lo55 = sum(1 for r in rows if r[1] < 0.55); lo40 = sum(1 for r in rows if r[1] < 0.40)
    steep = sum(1 for r in rows if r[2] > 55.01); off = sum(1 for r in rows if not r[3]); aoff = sum(1 for r in rows if not r[4]); outb = sum(1 for r in rows if not r[5])
    print(f'n {n} ice<55% {lo55/n:.1%} ice<40% {lo40/n:.1%} min {min(r[1] for r in rows):.2f} mean {sum(r[1] for r in rows)/n:.2f} | pitch>55 {steep/n:.1%} max {max(r[2] for r in rows):.1f} | chest off {off/n:.1%} | ahead3 off {aoff/n:.1%} | cam outside {outb}')
    for dd in (0.6, 1.0, 2.0, 3.0, 5.0, 7.0):
        rr = [r for r in rows if r[0] == dd]
        print(f'   d {dd}: ice<55% {sum(1 for r in rr if r[1] < 0.55)/len(rr):.1%} <40% {sum(1 for r in rr if r[1] < 0.40)/len(rr):.1%} min {min(r[1] for r in rr):.2f} pitch>55 {sum(1 for r in rr if r[2] > 55.01)/len(rr):.1%} chest off {sum(1 for r in rr if not r[3])/len(rr):.1%} ahead off {sum(1 for r in rr if not r[4])/len(rr):.1%}')
    return rows
if __name__ == '__main__':
    def clamped_rig(px, py, yaw):
        cam, p = rig(px, py, yaw); x, y = nearest_inside(cam[0], cam[1], MARGIN); return (x, y, cam[2]), p
    print('rig, clamped (the boards stages off)'); sweep(clamped_rig)
    P = dict(near=6.0, full=1.0, gate=8.0, gfull=4.0, slide=3.0, lower=True, hmin=2.0, cap=55.0, yt=0.45, keep=0.8)
    print('design', P); sweep(lambda x, y, a: mine(x, y, a, P))
