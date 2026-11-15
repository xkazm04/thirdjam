# Lane A214: picks the stills of the brief's twelve scenes from a run's JAMCAM lines (stills every 20 sim frames): four corners, behind each net, each side
# board, two hits at the boards (JAMKICK), two faceoffs (the drop: JAMMATCH start and after a goal). Prints frame, scene, his place.
import sys, re
log = sys.argv[1]
pat = re.compile(r'JAMCAM frame=(\d+) .*?px=(\S+) py=(\S+) .*?rl=(\S+) rw=(\S+)')
pos = {}; kicks = []
for line in open(log, encoding='utf-8', errors='replace'):
    m = pat.search(line)
    if m: f = int(m.group(1)); pos[f] = (float(m.group(2)), float(m.group(3)), float(m.group(4)), float(m.group(5))); continue
    k = re.search(r'JAMKICK frame=(\d+)', line)
    if k: kicks.append(int(k.group(1)))
st = [f for f in sorted(pos) if f % 20 == 0]
def best(name, cond):
    c = [f for f in st if cond(*pos[f])]
    print(name, c[:8])
best('corner +x+y', lambda x, y, L, W: x > L / 2 - 6 and y > W / 2 - 6)
best('corner +x-y', lambda x, y, L, W: x > L / 2 - 6 and y < -W / 2 + 6)
best('corner -x+y', lambda x, y, L, W: x < -L / 2 + 6 and y > W / 2 - 6)
best('corner -x-y', lambda x, y, L, W: x < -L / 2 + 6 and y < -W / 2 + 6)
best('behind net +x', lambda x, y, L, W: x > L / 2 - 4 and abs(y) < 4)
best('behind net -x', lambda x, y, L, W: x < -L / 2 + 4 and abs(y) < 4)
best('side +y', lambda x, y, L, W: y > W / 2 - 3 and abs(x) < L / 2 - 8)
best('side -y', lambda x, y, L, W: y < -W / 2 + 3 and abs(x) < L / 2 - 8)
def nearb(f):
    p = pos.get(f)
    return p and min(p[2] / 2 - abs(p[0]), p[3] / 2 - abs(p[1])) < 4
print('kicks near a board', [f for f in kicks if nearb(f)][:10], 'all kicks', kicks[:12])
