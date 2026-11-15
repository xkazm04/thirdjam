# Lane A211: what became of each pass, from a run's game.log (JAMPASS lines and the next JAMVIS takegrace on=1 line: who touched the puck next).
#   python passes.py <game.log>
import re, sys
ev = []
for l in open(sys.argv[1], encoding='utf-8', errors='replace'):
    m = re.search(r'JAMPASS frame=(\d+) skater=(\d+) team=(\d+) strength=([\d.]+) (\w+)', l)
    if m: ev.append((int(m.group(1)), 'pass', int(m.group(2)), int(m.group(3)), float(m.group(4)), m.group(5))); continue
    m = re.search(r'JAMVIS takegrace frame=(\d+) skater=(\d+) team=(\d+) human=(\d) on=1', l)
    if m: ev.append((int(m.group(1)), 'take', int(m.group(2)), int(m.group(3)), int(m.group(4)))); continue
    m = re.search(r'JAMSHOT frame=(\d+) skater=(\d+) team=(\d+)', l)
    if m: ev.append((int(m.group(1)), 'shot', int(m.group(2)), int(m.group(3)))); continue
    m = re.search(r'JAMRINK react goal end=(\S+) frame=(\d+)', l)
    if m: ev.append((int(m.group(2)), 'goal', m.group(1)))
ev.sort(key=lambda e: (e[0], 0 if e[1] == 'take' else 1))
out = {'mate': 0, 'self': 0, 'opp': 0, 'none': 0, 'goal': 0}
gaps = []; rows = []; tohuman = []
for i, e in enumerate(ev):
    if e[1] != 'pass': continue
    res, f2 = 'none', None
    for n in ev[i + 1:]:
        if n[0] - e[0] > 240: break
        if n[1] == 'goal': res, f2 = 'goal', n[0]; break
        if n[1] == 'take' and n[0] > e[0]:
            res = 'self' if n[2] == e[2] else ('mate' if n[3] == e[3] else 'opp'); f2 = n[0]
            if res == 'mate' and len(n) > 4 and n[4] == 1: tohuman.append((e[0], e[2], n[0]))
            break
        if n[1] == 'pass' and n[2] != e[2]:
            res = 'mate' if n[3] == e[3] else 'opp'; f2 = n[0]; break
    out[res] += 1
    if f2: gaps.append(f2 - e[0])
    rows.append((e[0], e[2], e[3], e[4], e[5], res, (f2 - e[0]) if f2 else -1))
n = sum(out.values())
print('passes', n, out)
for t in (0, 1):
    r = [x for x in rows if x[2] == t]
    print(' team', t, len(r), {k: sum(1 for x in r if x[5] == k) for k in out})
short = sum(1 for x in rows if 0 <= x[6] <= 15)
print(' next touch within 15 frames of the pass:', short, 'of', n)
print(' passes of his mates that the human took next (pass frame, passer, take frame):', tohuman)
if '-v' in sys.argv:
    for x in rows: print(' ', x)
