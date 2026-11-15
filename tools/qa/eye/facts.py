# Lane A211 (the eye): the countable facts of one filmed match, for the before / after of a list: one block per run.
#   python facts.py <run dir> [<run dir> ...]        (a run dir holds game.log and shots/)
# From the log: the score and the length, each faceoff and whether the human was knocked down within 60 frames of its drop, his knockdowns,
# his hit presses and what they did, his passes and what became of them, both sides' passes, every goal with the frames from the last shot to the
# goal (53 frames is a shot from the blue line), who shot it. From the stills: the camera's share of stands / no-horizon stills (view.py's rule) and
# the stills in which the yellow hint is painted over the button pill at the bottom left.
import sys, os, re
from PIL import Image

def facts(run):
    log = os.path.join(run, 'game.log')
    if not os.path.exists(log): print(run, 'no game.log'); return
    T = open(log, encoding='utf-8', errors='replace').read().splitlines()
    ev = []
    script = ''
    for l in T:
        m = re.search(r'JAMSCRIPT human=2 kind=(\w+)', l)
        if m: script = 'hand:' + m.group(1)
        m = re.search(r'JAMEVENT faceoff who=-1 .*frame=(\d+) kind=drop', l)
        if m: ev.append((int(m.group(1)), 'drop')); continue
        m = re.search(r'JAMVIS knockdown frame=(\d+) skater=(\d+) team=(\d+) human=(\d)', l)
        if m: ev.append((int(m.group(1)), 'kd', int(m.group(4)))); continue
        m = re.search(r'JAMHIT frame=(\d+) kind=(\w+) result=(\w+) checker=(\d+) victim=(-?\d+)', l)
        if m: ev.append((int(m.group(1)), 'hit', m.group(2), m.group(3), int(m.group(4)), int(m.group(5)))); continue
        m = re.search(r'JAMPASS frame=(\d+) skater=(\d+) team=(\d+) strength=([\d.]+) (\w+)', l)
        if m: ev.append((int(m.group(1)), 'pass', int(m.group(2)), int(m.group(3)), m.group(5))); continue
        m = re.search(r'JAMVIS takegrace frame=(\d+) skater=(\d+) team=(\d+) human=(\d) on=1', l)
        if m: ev.append((int(m.group(1)), 'take', int(m.group(2)), int(m.group(3)))); continue
        m = re.search(r'JAMSHOT frame=(\d+) skater=(\d+) team=(\d+) charge=([\d.]+) (\w+) one_timer=(\d)', l)
        if m: ev.append((int(m.group(1)), 'shot', int(m.group(2)), int(m.group(3)), m.group(5))); continue
        m = re.search(r'JAMRINK react goal end=(\S+) frame=(\d+)', l)
        if m: ev.append((int(m.group(2)), 'goal', m.group(1))); continue
        m = re.search(r'JAMSIM frame=(\d+) hash=\S+ score=(\d+):(\d+) phase=(\d)', l)
        if m: ev.append((int(m.group(1)), 'sim', int(m.group(2)), int(m.group(3)), int(m.group(4))))
    ev.sort(key=lambda e: (e[0], {'take': 0}.get(e[1], 1)))
    sims = [e for e in ev if e[1] == 'sim']
    goals = [e for e in ev if e[1] == 'goal']
    drops = [e[0] for e in ev if e[1] == 'drop']
    kd = [e[0] for e in ev if e[1] == 'kd' and e[2] == 1]
    kd_draw = [d for d in drops if any(d <= k <= d + 60 for k in kd)]
    last = sims[-1] if sims else None
    end = goals[-1][0] if goals else (last[0] if last else 0)
    print('== %s  %s' % (os.path.basename(run.rstrip('/\\')), script or 'old script'))
    if last: print('  score %d:%d, last goal at frame %d (%.0f s)' % (last[2], last[3], end, end / 60.0))
    print('  faceoffs %d, the human knocked down within 60 frames of the drop at %d: %s' % (len(drops), len(kd_draw), kd_draw))
    print('  the human knocked down %d times: %s' % (len(kd), kd))
    hh = [e for e in ev if e[1] == 'hit' and e[4] == 0]
    if hh:
        res = {}
        for e in hh: res[e[3]] = res.get(e[3], 0) + 1
        con = sum(v for k, v in res.items() if k in ('down', 'stumble', 'bump', 'strip'))
        print('  his hit presses %d, contacts %d: %s' % (len(hh), con, res))
    else: print('  his hit presses 0 (or the hit lines are off)')
    # passes
    rows = []
    for i, e in enumerate(ev):
        if e[1] != 'pass': continue
        res, gap = 'none', -1
        for n in ev[i + 1:]:
            if n[0] - e[0] > 240: break
            if n[1] == 'goal': res, gap = 'goal', n[0] - e[0]; break
            if n[1] == 'take' and n[0] > e[0]: res = 'self' if n[2] == e[2] else ('mate' if n[3] == e[3] else 'opp'); gap = n[0] - e[0]; break
            if n[1] == 'pass' and n[2] != e[2]: res = 'mate' if n[3] == e[3] else 'opp'; gap = n[0] - e[0]; break
        rows.append((e[0], e[2], e[3], e[4], res, gap))
    def line(tag, r):
        n = len(r)
        if not n: print('  %s passes 0' % tag); return
        c = lambda k: sum(1 for x in r if x[4] == k)
        back = sum(1 for x in r if x[4] == 'self' and 0 <= x[5] <= 10)
        quick = sum(1 for x in r if 0 <= x[5] <= 15)
        print('  %s passes %d: to a mate %d (%d %%), to an opponent %d, back to the passer %d (within 10 frames %d), touched again within 15 frames %d (%d %%)' %
              (tag, n, c('mate'), round(100 * c('mate') / n), c('opp'), c('self'), back, quick, round(100 * quick / n)))
    line('his', [x for x in rows if x[3] == 'human'])
    line('his side\'s', [x for x in rows if x[2] == 0])
    line('the other side\'s', [x for x in rows if x[2] == 1])
    line('all', rows)
    for g in goals:
        shots = [e for e in ev if e[1] == 'shot' and e[0] <= g[0] and g[0] - e[0] <= 90]
        s = shots[-1] if shots else None
        side = 'FOR' if g[2] == '+x' else 'against'
        print('  goal %s at %d: %s' % (side, g[0], ('shot at %d by skater %d (%s), %d frames before' % (s[0], s[2], s[4], g[0] - s[0])) if s else 'no shot in the 90 frames before (carried or deflected in)'))
    shots = os.path.join(run, 'shots')
    if os.path.isdir(shots):
        n = st = tp = ov = late = 0
        for name in sorted(os.listdir(shots)):
            m = re.match(r'^u1-.*-f0*(\d+)\.png$', name)
            if not m: continue
            f = int(m.group(1))
            if f % 20 or f < 500 or f > end: continue
            try: im = Image.open(os.path.join(shots, name)).convert('RGB')
            except Exception: continue
            sm = im.resize((160, 90)); px = list(sm.getdata())
            ok = lambda r, g, b: r > 125 and g > 110 and b > 115 and (r - g) < 34 and b > g - 6 and b > 0.82 * r
            ice = sum(ok(*p) for p in px) / len(px); top = sum(ok(*p) for p in px[:160 * 14]) / (160 * 14)
            n += 1
            if ice < 0.45: st += 1
            elif top > 0.4: tp += 1
            # the hint over the pill: yellow letters in the pill's box at the bottom left (1280 x 720: x 16..262, y 682..704)
            W, H = im.size
            box = im.crop((int(16 * W / 1280), int(682 * H / 720), int(262 * W / 1280), int(704 * H / 720)))
            yel = sum(1 for (r, g, b) in box.getdata() if r > 200 and g > 170 and b < 120)
            late += 1
            if yel > 40: ov += 1
        if n: print('  camera: %d stills (every 20 frames, 500 to the end): stands %d, no horizon %d = %d %%; the yellow hint over the pill in %d of %d stills' % (n, st, tp, round(100 * (st + tp) / n), ov, late))

for r in sys.argv[1:]: facts(r)
