# Lane A171 (track U1, evolution 6): how long the two ice marks are on screen. Reads the JAMHUD mark= lines of game logs (lane A166's UpdateMarks logs
# every change) and prints, per mark, every showing: the sim frames it began and ended, its length, whether it was drawn (shown=) and, for the contested
# ring, the seconds to the puck when it lit.   python tools/qa/screen/u6-marks.py <game.log> [<game.log> ...]
import os, re, sys
con = re.compile(r'JAMHUD frame=(\d+) mark=contested on=(\d) opp=(-?\d+) human_s=([\d.]+) opp_s=([\d.]+) closing=([\d.]+) shown=(\d)')
stop = re.compile(r'JAMHUD frame=(\d+) mark=stop state=(\w+) def=(-?\d+) dist=([\d.]+) closing=([\d.-]+) shown=(\d)')
allc, alls = [], []
for path in sys.argv[1:]:
    on = None; lit = None
    for line in open(path, 'r', encoding='utf-8', errors='replace'):
        m = con.search(line)
        if m:
            f = int(m.group(1))
            if m.group(2) == '1': on = (f, float(m.group(4)), float(m.group(5)), float(m.group(6)), m.group(7))
            elif on: allc.append((path, on[0], f, f - on[0], on[1], on[2], on[3], on[4])); on = None
        m = stop.search(line)
        if m:
            f = int(m.group(1))
            if m.group(2) == 'lit': lit = (f, m.group(6))
            elif lit: alls.append((path, lit[0], f, f - lit[0], lit[1])); lit = None
def show(name, rows, k):
    n = [r[3] for r in rows]
    if not n: print(name + ': none'); return
    n.sort()
    print('%s: %d showings; length in sim frames: shortest %d, median %d, longest %d; under 20 frames (a third of a second): %d; drawn on its first frame (shown=1): %d'
          % (name, len(n), n[0], n[len(n) // 2], n[-1], sum(1 for x in n if x < 20), sum(1 for r in rows if r[k] == '1')))
    for r in rows: print('   ', os.path.basename(os.path.dirname(r[0])), 'frames %d..%d (%d)' % (r[1], r[2], r[3]), ' '.join(str(x) for x in r[4:]))
show('contested ring', allc, 7)
show('stop mark, lit', alls, 4)
