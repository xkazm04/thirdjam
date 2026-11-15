# Lane A214: the puck behind him (READY 2 item 5). From a run's JAMCAM lines: stills (every 20th frame) with the puck within 15 m; of those behind his
# facing, how many show it; and the pull back (the camera moved back along its yaw, its height and pitch kept, and the same moved up along the line to keep
# him framed is NOT modelled) that would put it on screen, in metres.
import sys, re, math
from geom import frac, inside
pat = re.compile(r'JAMCAM frame=(\d+) .*?px=(\S+) py=(\S+) .*?fyaw=(\S+) fpitch=(\S+) fcx=(\S+) fcy=(\S+) fh=(\S+) pkx=(\S+) pky=(\S+) hyaw=(\S+) bw=(\S+) rl=(\S+) rw=(\S+) rr=(\S+)')
for log in sys.argv[1:]:
    rows = {}
    for line in open(log, encoding='utf-8', errors='replace'):
        m = pat.search(line)
        if m: v = [float(x) for x in m.groups()]; rows[int(v[0])] = v
    near = behind = off = offb = 0; pulls = []
    for f in sorted(rows):
        if f % 20: continue
        _, px, py, yaw, pitch, cx, cy, ch, pkx, pky, hyaw, bw, L, W, R = rows[f]
        d = math.hypot(pkx - px, pky - py)
        if d > 15 or d < 2.0: continue   # on his stick (< 2 m) is the carry, counted elsewhere
        near += 1
        on = lambda c: (lambda q: abs(q[0]) <= 1 and abs(q[1]) <= 1)(frac(c, yaw, pitch, (pkx, pky, 0.05)))
        a = math.radians(hyaw); isb = (pkx - px) * math.cos(a) + (pky - py) * math.sin(a) < 0
        behind += isb
        if not on((cx, cy, ch)):
            off += 1; offb += isb
            fy = math.radians(yaw); need = None
            for k in range(1, 61):
                b = k * 0.5; c = (cx - math.cos(fy) * b, cy - math.sin(fy) * b, ch)
                if on(c): need = b; break
            pulls.append(need)
    got = [p for p in pulls if p is not None]
    got.sort(); c2 = sum(1 for p in got if p <= 2.0); c3 = sum(1 for p in got if p <= 3.0)
    q = lambda t: got[min(len(got) - 1, int(t * (len(got) - 1)))] if got else None
    print(f'{log}: puck 2-15 m from him in {near} stills; behind his facing {behind}; off screen {off} (behind {offb}); pull back that shows it: '
          f'found {len(got)} of {len(pulls)}, median {q(0.5)} m, p90 {q(0.9)} m, max {got[-1] if got else None} m; within 2 m {c2}, within 3 m {c3}')
