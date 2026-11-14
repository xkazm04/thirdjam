# Lane A199 (track K1, evolution 3, item 5): the still-hand anchor's release, measured on A186's real-match camera logs (jam.camera.Log 1).
# FJamAimAnchor (lane A191) keeps the blade's offset from the skater while the cursor's pixel is still and the skater stands; the first frame the hand
# moves (or the skater skates) it re-projects at once, so whatever the camera corrected meanwhile arrives as one jump of the blade.
# Replayed here: every frame of every anchored stretch (the skater standing, the cursor still over a point 2.2 m ahead, 1.2 m right) is tried as
# the release frame (the hand moves one pixel). Numbers per release variant:
#   jump: the blade's turn on the release frame minus the turn the hand asked for (the one pixel re-projected), degrees: median / p90 / worst;
#   left after 0.25 s: the angle between the blade and the cursor's true point a quarter second after the release (the hand still again), degrees.
# Variants: 0 snap (A191, what main does); glide tau: the anchored-minus-true offset is carried and decays with time constant tau (the hand's own
# motion passes through at once).
#   python tools/qa/aim/a199-release.py <game.log> [...]
import math, sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import importlib.util
spec = importlib.util.spec_from_file_location('a191', os.path.join(os.path.dirname(os.path.abspath(__file__)), 'a191-aim.py'))
a191 = importlib.util.module_from_spec(spec); spec.loader.exec_module(a191)
proj, hit, ang, unwind, read, q = a191.proj, a191.hit, a191.ang, a191.unwind, a191.read, a191.q

STAND = 0.3


def stretches(rows):
    """Anchored stretches: consecutive frames with the skater under STAND m/s, at least 20 frames long."""
    frames = sorted(rows)
    out, cur = [], []
    for a, b in zip(frames, frames[1:]):
        ok = b == a + 1 and math.hypot(rows[b]['hm'][0] - rows[a]['hm'][0], rows[b]['hm'][1] - rows[a]['hm'][1]) * 60.0 < STAND
        if ok:
            if not cur: cur = [a]
            cur.append(b)
        else:
            if len(cur) >= 20: out.append(cur)
            cur = []
    if len(cur) >= 20: out.append(cur)
    return out


def measure(rows, taus):
    res = {t: dict(jump=[], left=[]) for t in taus}
    for st in stretches(rows):
        r0 = rows[st[0]]
        yr = math.radians(r0['rig'][3])
        ao = (math.cos(yr) * 2.2 - math.sin(yr) * 1.2, math.sin(yr) * 2.2 + math.cos(yr) * 1.2)
        pix = proj(r0['cam'], (r0['hm'][0] + ao[0], r0['hm'][1] + ao[1], 0))
        if not pix: continue
        h0 = hit(r0['cam'], pix)
        if not h0: continue
        anch = (h0[0] - r0['hm'][0], h0[1] - r0['hm'][1])   # the anchored offset (held through the stretch)
        for k in range(5, len(st) - 16, 3):
            f = st[k]
            rf, rp = rows[f], rows[f - 1]
            pix2 = (pix[0] + 1.0, pix[1])                      # the hand moves one pixel on frame f
            ht, hp = hit(rf['cam'], pix2), hit(rf['cam'], pix)
            if not ht or not hp: continue
            true_off = (ht[0] - rf['hm'][0], ht[1] - rf['hm'][1])
            if math.hypot(*true_off) < 0.7: continue
            asked = unwind(ang(true_off) - ang((hp[0] - rf['hm'][0], hp[1] - rf['hm'][1])))   # the pixel's own turn, re-projected
            for t in taus:
                corr = (anch[0] - true_off[0], anch[1] - true_off[1])   # the anchored point minus the true point at the release
                a = 1.0 if t <= 0 else 1.0 - math.exp(-(1.0 / 60.0) / t)
                # Release frame: the correction decays by one step; the hand's move passes through.
                corr = (corr[0] * (1 - a), corr[1] * (1 - a))
                got = (true_off[0] + corr[0], true_off[1] + corr[1])
                jump = abs(unwind(unwind(ang(got) - ang(anch)) - asked))
                res[t]['jump'].append(jump)
                # 0.25 s later, the hand still on pix2: the correction keeps decaying.
                g = f + 15
                if g in rows:
                    hg = hit(rows[g]['cam'], pix2)
                    if hg:
                        tg = (hg[0] - rows[g]['hm'][0], hg[1] - rows[g]['hm'][1])
                        c2 = (corr[0] * (1 - a) ** 15, corr[1] * (1 - a) ** 15)
                        res[t]['left'].append(abs(unwind(ang((tg[0] + c2[0], tg[1] + c2[1])) - ang(tg))))
    return res


def main():
    taus = [0.0, 0.05, 0.1, 0.15, 0.25]
    print('| log | release | n | jump median / p90 / worst deg | left after 0.25 s median / p90 / worst deg |')
    print('|---|---|---|---|---|')
    for path in sys.argv[1:]:
        res = measure(read(path), taus)
        name = os.path.basename(os.path.dirname(os.path.dirname(path)))
        for t in taus:
            j, l = sorted(res[t]['jump']), sorted(res[t]['left'])
            lab = 'snap (main)' if t <= 0 else f'glide tau {t:.2f} s'
            print(f"| {name} | {lab} | {len(j)} | {q(j, 0.5):.1f} / {q(j, 0.9):.1f} / {q(j, 1.0):.1f} | {q(l, 0.5):.1f} / {q(l, 0.9):.1f} / {q(l, 1.0):.1f} |")


if __name__ == '__main__':
    main()
