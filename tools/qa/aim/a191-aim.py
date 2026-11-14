# Lane A191 (track K1, evolution 2): a still hand's aim, measured on a real match's camera log (jam.camera.Log 1, lane A186's full-precision pose).
# The game aims the blade at the cursor's ice point: the cursor pixel re-projected through the camera every sim step (AHokejGameMode::CursorAimRink).
# Variants of jam.ctrl.AimAnchor, replayed on the logged camera and skater (the same arithmetic as FJamAimAnchor::Step in InputMapping.cpp):
#   0 reproject (today); 1 anchor (a still pixel keeps the aim's offset from the skater); 2 anchor only while the camera's correction moves the
#   picture, else reproject; 3 catch-up (a still pixel's offset closes on the re-projected one with time constant tau).
# Two numbers, both under a still hand:
#   A. the blade's turn under a still cursor through corrections (u7-bound.py's metric: frames where the skater is still and the camera corrects;
#      an episode's largest summed turn), degrees;
#   B. the drift in play: the angle between the blade's aim and the cursor's true ice point (what the pixel shows) after the hand stopped, at 0.25, 0.5,
#      1 and 2 s, median / p90 / worst over hand-stops every 15 frames.
#   python tools/qa/aim/a191-aim.py <game.log> [--json out.json]
import json, math, re, sys

PAT = re.compile(r'JAMCAM frame=(\d+) .*?px=(-?[\d.]+) py=(-?[\d.]+) .*?rx=(-?[\d.]+) ry=(-?[\d.]+) ryaw=(-?[\d.]+) bg=(\d) fyaw=(-?[\d.]+) fpitch=(-?[\d.]+) fcx=(-?[\d.]+) fcy=(-?[\d.]+) fh=(-?[\d.]+)')
FOV, W, H = 70.0, 1920.0, 1080.0
TANH = math.tan(math.radians(FOV / 2)); TANV = TANH / (W / H)
RIG_H, RIG_PITCH = 3.5, -math.degrees(math.atan(3.5 / 11.0))


def axes(yaw, pitch):
    y, p = math.radians(yaw), math.radians(pitch)
    return ((math.cos(p) * math.cos(y), math.cos(p) * math.sin(y), math.sin(p)), (-math.sin(y), math.cos(y), 0.0),
            (-math.sin(p) * math.cos(y), -math.sin(p) * math.sin(y), math.cos(p)))


def proj(pose, pt):
    cx, cy, cz, yaw, pitch = pose
    f, r, u = axes(yaw, pitch)
    d = (pt[0] - cx, pt[1] - cy, pt[2] - cz)
    lx = sum(a * b for a, b in zip(d, f)); ly = sum(a * b for a, b in zip(d, r)); lz = sum(a * b for a, b in zip(d, u))
    if lx <= 1e-3: return None
    fx, fy = ly / lx / TANH, lz / lx / TANV
    if abs(fx) > 1 or abs(fy) > 1: return None
    return (fx * W / 2, -fy * H / 2)


def hit(pose, pix):
    cx, cy, cz, yaw, pitch = pose
    f, r, u = axes(yaw, pitch)
    lx, ly, lz = 1.0, pix[0] / (W / 2) * TANH, -pix[1] / (H / 2) * TANV
    d = tuple(lx * f[i] + ly * r[i] + lz * u[i] for i in range(3))
    if d[2] >= -1e-4: return None
    t = -cz / d[2]
    return (cx + d[0] * t, cy + d[1] * t)


def ang(v): return math.degrees(math.atan2(v[1], v[0]))


def unwind(a):
    while a > 180: a -= 360
    while a < -180: a += 360
    return a


def read(path):
    rows = {}
    for line in open(path, encoding='utf-8', errors='replace'):
        m = PAT.search(line)
        if not m: continue
        g = m.groups()
        rows[int(g[0])] = dict(cam=(float(g[9]), float(g[10]), float(g[11]), float(g[7]), float(g[8])), hm=(float(g[1]), float(g[2])),
                               rig=(float(g[3]), float(g[4]), RIG_H, float(g[5]), RIG_PITCH))
    return rows


def correction_px(ra, rb):
    """The camera's correction: 16 ice points fixed to the skater, camera image motion minus the open-ice rig's, the worst in px."""
    worst = 0.0
    for i in range(16):
        a, rad = i * math.pi / 4, (2.5 if i < 8 else 5.0)
        o = (math.cos(a) * rad, math.sin(a) * rad)
        p0 = proj(ra['cam'], (ra['hm'][0] + o[0], ra['hm'][1] + o[1], 0)); p1 = proj(rb['cam'], (rb['hm'][0] + o[0], rb['hm'][1] + o[1], 0))
        q0 = proj(ra['rig'], (ra['hm'][0] + o[0], ra['hm'][1] + o[1], 0)); q1 = proj(rb['rig'], (rb['hm'][0] + o[0], rb['hm'][1] + o[1], 0))
        if not (p0 and p1 and q0 and q1): continue
        worst = max(worst, math.hypot((p1[0] - p0[0]) - (q1[0] - q0[0]), (p1[1] - p0[1]) - (q1[1] - q0[1])))
    return worst


class Aim:
    def __init__(s, mode, tau): s.mode, s.tau, s.off = mode, tau, None

    def step(s, true_off, corr, moved, speed=0.0):
        """true_off: the re-projected ice point minus the skater (what the game aims at today). Returns the offset aimed at."""
        if s.mode == 0 or s.off is None or moved:
            s.off = true_off; return s.off
        if s.mode == 4:   # anchor while the skater stands (speed under tau m/s), else reproject
            if speed >= s.tau: s.off = true_off
            return s.off
        if s.mode == 1: return s.off
        if s.mode == 2:
            if not corr: s.off = true_off
            return s.off
        a = 1.0 - math.exp(-(1.0 / 60.0) / s.tau)
        s.off = (s.off[0] + (true_off[0] - s.off[0]) * a, s.off[1] + (true_off[1] - s.off[1]) * a)
        return s.off


def q(xs, p): return xs[min(len(xs) - 1, int(p * (len(xs) - 1) + 0.5))] if xs else 0.0


def run(rows, variants, corr_px):
    frames = sorted(rows)
    cpx = {}
    for a, b in zip(frames, frames[1:]):
        if b == a + 1: cpx[b] = correction_px(rows[a], rows[b])
    corr = {f: v > corr_px for f, v in cpx.items()}
    out = {}
    for name, (mode, tau) in variants.items():
        # A: the skater still, the camera corrects, the cursor held at the pixel over a point 2.5 m ahead-right (u7-bound.py's point).
        episodes, cum, cmax, last = [], 0.0, 0.0, -100
        aim, prev, pix = None, None, None
        for a, b in zip(frames, frames[1:]):
            if b != a + 1: pix = None; continue
            ra, rb = rows[a], rows[b]
            step_m = math.hypot(rb['hm'][0] - ra['hm'][0], rb['hm'][1] - ra['hm'][1])
            still = step_m < 0.5 / 60.0 and abs(unwind(rb['rig'][3] - ra['rig'][3])) < 0.05
            if not still: pix = None; continue
            if pix is None:
                yr = math.radians(ra['rig'][3])
                ao = (math.cos(yr) * 2.2 - math.sin(yr) * 1.2, math.sin(yr) * 2.2 + math.cos(yr) * 1.2)
                pix = proj(ra['cam'], (ra['hm'][0] + ao[0], ra['hm'][1] + ao[1], 0))
                if not pix: continue
                h = hit(ra['cam'], pix)
                if not h: pix = None; continue
                aim = Aim(mode, tau)
                prev = ang(aim.step((h[0] - ra['hm'][0], h[1] - ra['hm'][1]), corr.get(a, False), True))
            h = hit(rb['cam'], pix)
            if not h: pix = None; continue
            cur = ang(aim.step((h[0] - rb['hm'][0], h[1] - rb['hm'][1]), corr.get(b, False), False, step_m * 60.0))
            turn = unwind(cur - prev); prev = cur
            if cpx.get(b, 0.0) > 0.5:
                if b - last > 10:
                    if last > -100: episodes.append(cmax)
                    cum, cmax = 0.0, 0.0
                last = b; cum += turn; cmax = max(cmax, abs(cum))
        if last > -100: episodes.append(cmax)
        episodes.sort()
        # B: in play: the hand stops every 15 frames and stays still for 2 s; the angle between the aim and the cursor's true ice point.
        at = {15: [], 30: [], 60: [], 120: []}
        worst2 = []
        for s0 in frames[::15]:
            r0 = rows[s0]
            yr = math.radians(r0['rig'][3])
            ao = (math.cos(yr) * 2.2 - math.sin(yr) * 1.2, math.sin(yr) * 2.2 + math.cos(yr) * 1.2)
            px0 = proj(r0['cam'], (r0['hm'][0] + ao[0], r0['hm'][1] + ao[1], 0))
            if not px0: continue
            h = hit(r0['cam'], px0)
            if not h: continue
            am = Aim(mode, tau)
            am.step((h[0] - r0['hm'][0], h[1] - r0['hm'][1]), corr.get(s0, False), True)
            vals = {}
            for k in range(1, 121):
                f = s0 + k
                if f not in rows or math.hypot(rows[f]['hm'][0] - rows[f - 1]['hm'][0], rows[f]['hm'][1] - rows[f - 1]['hm'][1]) > 5.0: break
                rf = rows[f]
                h = hit(rf['cam'], px0)
                if not h: break
                tru = (h[0] - rf['hm'][0], h[1] - rf['hm'][1])
                got = am.step(tru, corr.get(f, False), False, math.hypot(rows[f]['hm'][0] - rows[f - 1]['hm'][0], rows[f]['hm'][1] - rows[f - 1]['hm'][1]) * 60.0)
                if math.hypot(*tru) > 0.7: vals[k] = abs(unwind(ang(got) - ang(tru)))
            for k in at:
                if k in vals: at[k].append(vals[k])
        for v in at.values(): v.sort()
        out[name] = dict(A_episodes=len(episodes), A_median=q(episodes, 0.5), A_p90=q(episodes, 0.9), A_worst=q(episodes, 1.0),
                         B={f'{k / 60:.2f}s': dict(n=len(v), median=q(v, 0.5), p90=q(v, 0.9), worst=q(v, 1.0)) for k, v in at.items()})
    ncorr = sum(1 for v in corr.values() if v)
    return out, ncorr, len(corr)


def main():
    args = sys.argv[1:]
    js, cp = None, 0.5
    if '--json' in args: i = args.index('--json'); js = args[i + 1]; del args[i:i + 2]
    if '--corr-px' in args: i = args.index('--corr-px'); cp = float(args[i + 1]); del args[i:i + 2]
    rows = read(args[0])
    variants = {'0 reproject': (0, 0), '1 anchor': (1, 0), '2 anchor while correcting': (2, 0), '3 catch-up tau 0.25 s': (3, 0.25),
                '3 catch-up tau 0.5 s': (3, 0.5), '3 catch-up tau 1 s': (3, 1.0), '3 catch-up tau 2 s': (3, 2.0),
                '4 anchor while he stands (< 0.3 m/s)': (4, 0.3), '4 anchor while he stands (< 1 m/s)': (4, 1.0), '4 anchor while he stands (< 2 m/s)': (4, 2.0)}
    res, nc, nf = run(rows, variants, cp)
    print(f'{args[0]}: {len(rows)} frames, correction frames (over {cp} px) {nc} of {nf}')
    print('| variant | A: still-cursor turn through corrections, median / p90 / worst deg (episodes) | B drift 0.25 s | 0.5 s | 1 s | 2 s (median / p90 / worst deg) |')
    print('|---|---|---|---|---|---|')
    for n, r in res.items():
        b = ' | '.join(f"{v['median']:.1f} / {v['p90']:.1f} / {v['worst']:.1f}" for v in r['B'].values())
        print(f"| {n} | {r['A_median']:.1f} / {r['A_p90']:.1f} / {r['A_worst']:.1f} ({r['A_episodes']}) | {b} |")
    if js: json.dump(res, open(js, 'w'), indent=1)


if __name__ == '__main__':
    main()
