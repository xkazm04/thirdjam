# Lane A211 (the eye): a table of headless seed runs (eye-hash's out/<run>/hashes.txt and game.log) for the curve lane and for the before / after.
#   python seeds.py <prefix> [first] [last]      e.g. python seeds.py c-pro- 1 7   (reads out/c-pro-1 .. out/c-pro-7)
# Per run: the hashes at f600 and f3600, the score at the last JAMSIM line with its frame and whether the match was over (phase 2), the frame of
# the first goal against, his knockdowns, his aimed tap passes that arrived, his hit contacts, the goals against by frames from the shot.
import sys, os, re
pre = sys.argv[1]; a = int(sys.argv[2]) if len(sys.argv) > 2 else 1; b = int(sys.argv[3]) if len(sys.argv) > 3 else 7
here = os.path.dirname(os.path.abspath(__file__))
tot = dict(n=0, won=0, lost=0, open=0, first=0, kd=0, ptar=0, parr=0, pall=0, hp=0, hc=0, ga=0, ga43=0, ga51=0, own=0)
print('| Seed | Run | f600 | f3600 | Score (frame) | Over | First goal against | Floored | His aimed taps: arrived / with a man on the line | His hits: contacts / presses | Goals against: frames from the shot |')
print('|---|---|---|---|---|---|---|---|---|---|---|')
for s in range(a, b + 1):
    run = os.path.join(here, 'out', pre + str(s)); log = os.path.join(run, 'game.log')
    if not os.path.exists(log): print('| %d | %s | missing |' % (s, pre + str(s))); continue
    h = {}; last = None; ev = []
    for l in open(log, encoding='utf-8', errors='replace'):
        m = re.search(r'JAMSIM frame=(\d+) hash=(\S+) score=(\d+):(\d+) phase=(\d)', l)
        if m: h[int(m.group(1))] = m.group(2); last = (int(m.group(1)), int(m.group(3)), int(m.group(4)), int(m.group(5))); continue
        m = re.search(r'JAMEVENT pass who=0 .*frame=(\d+) kind=(\S+)', l)
        if m: ev.append((int(m.group(1)), 'hp', m.group(2))); continue
        m = re.search(r'JAMPASS frame=(\d+) skater=(\d+) team=(\d+)', l)
        if m: ev.append((int(m.group(1)), 'pass', int(m.group(2)), int(m.group(3)))); continue
        m = re.search(r'JAMVIS takegrace frame=(\d+) skater=(\d+) team=(\d+) human=(\d) on=1', l)
        if m: ev.append((int(m.group(1)), 'take', int(m.group(2)), int(m.group(3)))); continue
        m = re.search(r'JAMVIS knockdown frame=(\d+) skater=\d+ team=\d+ human=1', l)
        if m: ev.append((int(m.group(1)), 'kd')); continue
        m = re.search(r'JAMHIT frame=(\d+) kind=(\w+) result=(\w+) checker=0 ', l)
        if m: ev.append((int(m.group(1)), 'hit', m.group(3))); continue
        m = re.search(r'JAMSHOT frame=(\d+) skater=(\d+) team=(\d+)', l)
        if m: ev.append((int(m.group(1)), 'shot', int(m.group(2)), int(m.group(3)))); continue
        m = re.search(r'JAMRINK react goal end=(\S+) frame=(\d+)', l)
        if m: ev.append((int(m.group(2)), 'goal', m.group(1)))
    ev.sort(key=lambda e: (e[0], 0 if e[1] == 'take' else 1))
    kd = sum(1 for e in ev if e[1] == 'kd')
    hits = [e for e in ev if e[1] == 'hit']; hc = sum(1 for e in hits if e[2] in ('down', 'stumble', 'bump', 'strip'))
    ga = []; first = None; own = 0
    for i, e in enumerate(ev):
        if e[1] != 'goal' or e[2] == '+x': continue
        sh = [x for x in ev[:i] if x[1] == 'shot' and e[0] - x[0] <= 240]
        if first is None: first = e[0]
        if sh: ga.append(e[0] - sh[-1][0])
        else: own += 1; ga.append(-1)
    tar = arr = alln = 0
    for i, e in enumerate(ev):
        if e[1] != 'hp': continue
        alln += 1
        if e[2].endswith('_-1'): continue
        tar += 1
        for n in ev[i + 1:]:
            if n[0] - e[0] > 240 or n[1] == 'goal': break
            if n[1] == 'take' and n[0] > e[0]:
                if n[2] != 0 and n[3] == 0: arr += 1
                break
            if n[1] == 'pass' and n[2] != 0:
                if n[3] == 0: arr += 1
                break
    over = last and last[3] == 2
    res = '' if not last else ('won' if over and last[1] > last[2] else 'lost' if over else 'open')
    tot['n'] += 1; tot[res if res in ('won', 'lost') else 'open'] += 1
    tot['kd'] += kd; tot['ptar'] += tar; tot['parr'] += arr; tot['pall'] += alln; tot['hp'] += len(hits); tot['hc'] += hc
    tot['ga'] += len([g for g in ga if g >= 0]); tot['ga43'] += len([g for g in ga if g >= 43]); tot['ga51'] += len([g for g in ga if g >= 51]); tot['own'] += own
    if first is not None and first <= 600: tot['first'] += 1
    print('| %d | `%s` | %s | %s | %d:%d (%d) | %s | %s | %d | %d / %d | %d / %d | %s |' % (s, pre + str(s), h.get(600, '-'), h.get(3600, '-'), last[1], last[2], last[0], res,
          first if first is not None else '-', kd, arr, tar, hc, len(hits), ' '.join(str(g) if g >= 0 else 'none' for g in ga) or '-'))
t = tot
print('\nSum of %d: won %d, lost %d, open %d; first goal against inside 600 frames in %d; floored %d times; aimed taps arrived %d of %d (all his passes %d); hits %d contacts of %d presses; goals against with a shot %d (%d at 43 frames or more, %d at 51 or more), without a shot in 90 frames %d' %
      (t['n'], t['won'], t['lost'], t['open'], t['first'], t['kd'], t['parr'], t['ptar'], t['pall'], t['hc'], t['hp'], t['ga'], t['ga43'], t['ga51'], t['own']))
