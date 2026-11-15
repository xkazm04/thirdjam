# Lane A214: the C++ BoardsViewTarget mirrored (no spring), to look at single poses.
import math
from design import *
def target(px, py, yaw, P):
    a = math.radians(yaw); f = (math.cos(a), math.sin(a)); r = (-f[1], f[0])
    ux = uy = 0.0; dmin = 1e9
    for d, n in walls(px, py):
        w = ss((P['near'] - d) / (P['near'] - P['full'])); ux -= w * n[0]; uy -= w * n[1]; dmin = min(dmin, d)
    wgt = ss((P['gate'] - dmin) / (P['gate'] - P['gfull']))
    s = P['slide'] * (ux * r[0] + uy * r[1])
    wx, wy = px - D * f[0] + (OFF + s) * r[0], py - D * f[1] + (OFF + s) * r[1]
    m = P['margin']
    if inside(wx, wy, L, W, R, m): fx, fy = wx, wy
    else:
        ox, oy = nearest_inside(px, py, m); lo, hi = 0.0, 1.0
        for _ in range(20):
            mid = (lo + hi) / 2
            if inside(ox + (wx - ox) * mid, oy + (wy - oy) * mid, L, W, R, m): lo = mid
            else: hi = mid
        fx, fy = ox + (wx - ox) * lo, oy + (wy - oy) * lo
    rho = (px - fx) * f[0] + (py - fy) * f[1]
    t = max(0.0, min(1.0, rho / P['short']))
    h = min(H0, P['hmin'] + (H0 - P['hmin']) * t); zs = 1.0 + (P['head'] - 1.0) * (1 - t)
    c = ray(fx, fy, a)
    drig = math.degrees(math.atan(h / max(rho + LA, 1e-3)))
    dice = math.degrees(math.atan(h / max(c, 1e-3)) + math.atan(P['yt'] * TANV))
    toh = math.degrees(math.atan2(h - zs, max(rho, 0.05))); e = math.degrees(math.atan(P['keep'] * TANV))
    tob = math.degrees(math.atan2(h - 0.05, max(rho + 0.9, 0.05)))
    lo, hi = max(tob, toh) - e, min(tob, toh) + e
    down = max(drig, dice)
    down = max(lo, min(hi, down)) if lo <= hi else 0.5 * (lo + hi)
    down = min(down, P['cap'])
    rx, rp = rig(px, py, yaw)
    cam = (rx[0] + wgt * (fx - rx[0]), rx[1] + wgt * (fy - rx[1]), rx[2] + wgt * (h - rx[2]))
    return cam, -(-rp + wgt * (down + rp))
P0 = dict(near=6.0, full=1.0, gate=8.0, gfull=4.0, slide=3.0, margin=0.4, hmin=2.2, short=4.0, head=1.8, cap=55.0, yt=0.45, keep=0.85)
if __name__ == '__main__':
    import sys
    P = dict(P0)
    for a in sys.argv[1:]: k, v = a.split('='); P[k] = float(v)
    for d in (0.5, 1.0, 2.0):
        py = W / 2 - d
        for yaw in range(-180, 180, 30):
            cam, p = target(0.0, py, yaw, P)
            fc = frac(cam, yaw, p, (0, py, 1.0)); fh = frac(cam, yaw, p, (0, py, 1.8))
            a = math.radians(yaw); pk = frac(cam, yaw, p, (0.9 * math.cos(a), py + 0.9 * math.sin(a), 0.05))
            bad = lambda q: abs(q[0]) > 1 or abs(q[1]) > 1
            print(f'd {d} yaw {yaw:4d} cam ({cam[0]:6.2f},{cam[1]:6.2f},{cam[2]:.2f}) down {-p:5.1f} chest ({fc[0]:5.2f},{fc[1]:5.2f}) head ({fh[0]:5.2f},{fh[1]:5.2f}) puck ({pk[0]:5.2f},{pk[1]:5.2f}) {"CHEST" if bad(fc) else ""} {"PUCK" if bad(pk) else ""}')
