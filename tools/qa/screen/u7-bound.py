# Lane A186 (track U1, evolution 7): the boards' corrections in a real match, from the camera's own log (jam.camera.Log 1: one JAMCAM line a sim frame).
# For every pair of consecutive sim frames: the picture of 16 ice points fixed to the human (2.5 and 5 m around him), in pixels at 1920 x 1080, as the
# camera moved them, minus what the open-ice rig of the same frames did (the log's rx= ry= ryaw= fields, lane A186; without them only the frames with
# the human still are read, where the rig stands still too). The cursor held still at the pixel over a point 2.5 m ahead-right of him: the blade's turn
# the camera added (the game's CursorAimFromRay: the ray onto the ice, the blade from him to the hit).
#   python tools\qa\screen\u7-bound.py <game.log> [<game.log> ...] [--k 3] [--json out.json]
import json, math, re, sys

PAT = re.compile(r'JAMCAM frame=(\d+) .*?yaw=(-?[\d.]+) pitch=(-?[\d.]+) .*?h=(-?[\d.]+) px=(-?[\d.]+) py=(-?[\d.]+) .*?cx=(-?[\d.]+) cy=(-?[\d.]+)(?: rx=(-?[\d.]+) ry=(-?[\d.]+) ryaw=(-?[\d.]+) bg=(\d) fyaw=(-?[\d.]+) fpitch=(-?[\d.]+) fcx=(-?[\d.]+) fcy=(-?[\d.]+) fh=(-?[\d.]+))?')
FOV, W, H = 70.0, 1920.0, 1080.0
TANH = math.tan(math.radians(FOV / 2)); TANV = TANH / (W / H)
RIG_H, RIG_PITCH = 3.5, -math.degrees(math.atan(3.5 / 11.0))   # the cartridge rig: height 3.5, distance 6 + look-ahead 5


def axes(yaw, pitch):
    y, p = math.radians(yaw), math.radians(pitch)
    f = (math.cos(p) * math.cos(y), math.cos(p) * math.sin(y), math.sin(p))
    r = (-math.sin(y), math.cos(y), 0.0)
    u = (-math.sin(p) * math.cos(y), -math.sin(p) * math.sin(y), math.cos(p))
    return f, r, u


def proj(pose, pt):
    (cx, cy, cz, yaw, pitch) = pose
    f, r, u = axes(yaw, pitch)
    d = (pt[0] - cx, pt[1] - cy, pt[2] - cz)
    lx = sum(a * b for a, b in zip(d, f)); ly = sum(a * b for a, b in zip(d, r)); lz = sum(a * b for a, b in zip(d, u))
    if lx <= 1e-3:
        return None
    fx, fy = ly / lx / TANH, lz / lx / TANV
    if abs(fx) > 1 or abs(fy) > 1:
        return None
    return (fx * W / 2, -fy * H / 2)


def blade(pose, pix, hm):
    (cx, cy, cz, yaw, pitch) = pose
    f, r, u = axes(yaw, pitch)
    lx, ly, lz = 1.0, pix[0] / (W / 2) * TANH, -pix[1] / (H / 2) * TANV
    d = tuple(lx * f[i] + ly * r[i] + lz * u[i] for i in range(3))
    if d[2] >= -1e-4:
        return None
    t = -cz / d[2]
    hx, hy = cx + d[0] * t - hm[0], cy + d[1] * t - hm[1]
    if math.hypot(hx, hy) < 0.5:
        return None
    return math.degrees(math.atan2(hy, hx))


def unwind(a):
    while a > 180: a -= 360
    while a < -180: a += 360
    return a


def read(path):
    rows = {}
    with open(path, encoding='utf-8', errors='replace') as fh:
        for line in fh:
            m = PAT.search(line)
            if not m:
                continue
            g = m.groups()
            fr = int(g[0])
            cam = (float(g[6]), float(g[7]), float(g[3]), float(g[1]), float(g[2]))
            hm = (float(g[4]), float(g[5]))
            rig = None
            if g[12] is not None:   # lane A186's full-precision pose
                cam = (float(g[14]), float(g[15]), float(g[16]), float(g[12]), float(g[13]))
            if g[8] is not None:
                rig = (float(g[8]), float(g[9]), RIG_H, float(g[10]), RIG_PITCH)
            rows[fr] = (cam, hm, rig, int(g[11]) if g[11] else 0)   # the last line of a frame wins (the log may repeat frame 0)
    return rows


def analyse(rows, k, only=None):
    frames = sorted(f for f in rows if only is None or f in only)
    st = dict(frames=0, still=0, over=0, over_still=0, worst=0.0, worst_still=0.0, episodes=0, reversals=0, blade_frame=0.0, guard=0)
    blades, last_corr, cum, cmax = [], -100, 0.0, 0.0
    last_sig, last_sig_f = None, -100
    worst_at = None
    for a, b in zip(frames, frames[1:]):
        if b != a + 1:
            continue
        (ca, ha, ra, _), (cb, hb, rb, gb) = rows[a], rows[b]
        if math.hypot(hb[0] - ha[0], hb[1] - ha[1]) > 5.0:   # a faceoff's teleport
            continue
        st['frames'] += 1
        if gb: st['guard'] += 1
        still = math.hypot(hb[0] - ha[0], hb[1] - ha[1]) < 0.5 / 60.0 and (ra is None or rb is None or abs(unwind(rb[3] - ra[3])) < 0.05)   # no move, no turn (the rig's yaw holds)
        if not still and (ra is None or rb is None):
            continue
        if still: st['still'] += 1
        worst = 0.0
        for i in range(16):
            ang, rad = i * math.pi / 4, (2.5 if i < 8 else 5.0)
            o = (math.cos(ang) * rad, math.sin(ang) * rad)
            p0 = proj(ca, (ha[0] + o[0], ha[1] + o[1], 0)); p1 = proj(cb, (hb[0] + o[0], hb[1] + o[1], 0))
            if not p0 or not p1:
                continue
            dx, dy = p1[0] - p0[0], p1[1] - p0[1]
            if ra and rb:
                q0 = proj(ra, (ha[0] + o[0], ha[1] + o[1], 0)); q1 = proj(rb, (hb[0] + o[0], hb[1] + o[1], 0))
                if not q0 or not q1:
                    continue
                dx -= q1[0] - q0[0]; dy -= q1[1] - q0[1]
            worst = max(worst, math.hypot(dx, dy))
        # the held cursor
        yr = math.radians(ra[3] if ra else ca[3])
        ao = (math.cos(yr) * 2.2 - math.sin(yr) * 1.2, math.sin(yr) * 2.2 + math.cos(yr) * 1.2)
        bc, aim = 0.0, None
        c = proj(ca, (ha[0] + ao[0], ha[1] + ao[1], 0))
        if c:
            a0, a1 = blade(ca, c, ha), blade(cb, c, hb)
            if a0 is not None and a1 is not None:
                bc = unwind(a1 - a0)
                if ra and rb:
                    cr = proj(ra, (ha[0] + ao[0], ha[1] + ao[1], 0))
                    r0, r1 = (blade(ra, cr, ha), blade(rb, cr, hb)) if cr else (None, None)
                    if r0 is not None and r1 is not None:
                        bc -= unwind(r1 - r0)
            p1 = proj(cb, (hb[0] + ao[0], hb[1] + ao[1], 0))
            if p1:
                aim = (p1[0] - c[0], p1[1] - c[1])
        st['blade_frame'] = max(st['blade_frame'], abs(bc))
        if worst > st['worst']:
            st['worst'] = worst; worst_at = b
        if still: st['worst_still'] = max(st['worst_still'], worst)
        if worst > k:
            st['over'] += 1
            if still: st['over_still'] += 1
        if still and worst > 0.5:   # a correction with him still: the blade's turn under the held cursor summed over it
            if b - last_corr > 10:
                if last_corr > -100: blades.append(cmax)
                st['episodes'] += 1; cum = 0.0; cmax = 0.0
            last_corr = b
            cum += bc; cmax = max(cmax, abs(cum))
        if aim and math.hypot(*aim) > 1.0 and (ra is None or still):
            if last_sig and b - last_sig_f <= 30 and aim[0] * last_sig[0] + aim[1] * last_sig[1] < 0:
                st['reversals'] += 1
            last_sig, last_sig_f = aim, b
    if last_corr > -100: blades.append(cmax)
    blades.sort()
    q = lambda p: blades[min(len(blades) - 1, int(p * (len(blades) - 1) + 0.5))] if blades else 0.0
    st.update(blade_median=q(0.5), blade_p90=q(0.9), blade_worst=q(1.0), worst_at=worst_at, rig=any(r[2] is not None for r in rows.values()))
    return st


def main():
    args = sys.argv[1:]
    k, out = 3.0, None
    if '--k' in args: i = args.index('--k'); k = float(args[i + 1]); del args[i:i + 2]
    if '--json' in args: i = args.index('--json'); out = args[i + 1]; del args[i:i + 2]
    if '--common' in args: args.remove('--common')
    res = {}
    logs = {p: read(p) for p in args}
    common = set.intersection(*[set(r) for r in logs.values()]) if '--common' in sys.argv else None
    for p in args:
        st = analyse(logs[p], k, common)
        res[p] = st
        print(f"{p}: frames {st['frames']} (still {st['still']}, rig in log: {st['rig']}) | episodes {st['episodes']} | over {k:g} px {st['over']} (still {st['over_still']}) | "
              f"worst {st['worst']:.1f} px/frame at sim frame {st['worst_at']}, still {st['worst_still']:.1f} | reversals {st['reversals']} | blade a frame {st['blade_frame']:.1f} deg; "
              f"a still correction median {st['blade_median']:.1f}, p90 {st['blade_p90']:.1f}, worst {st['blade_worst']:.1f} | guard frames {st['guard']}")
    if out:
        json.dump(res, open(out, 'w'), indent=1)


if __name__ == '__main__':
    main()
