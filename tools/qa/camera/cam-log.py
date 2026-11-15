# Lane A214 (track U1): the game camera of a real match, frame by frame, from its JAMCAM lines (jam.camera.Log 1), counted as the eye counts a still.
#   python cam-log.py <game.log> [every=20] [from=0] [label]
# Per sim frame: the share of the 16:9 frame that is ice inside the boards' inner line (rays onto z = 0, geom.ice_share), the pitch, his chest (and head,
# skates) on screen, the puck on screen when within 20 m, 3 m of ice in his direction of travel on screen, the eye inside the line (0.3 m: not in the glass),
# the camera's own turn over 30 frames beyond his. Counted on every `every`-th frame (the eye's still cadence) and on the frames within 5 m of a board.
import sys, re, math
from geom import ice_share, frac, inside
log = sys.argv[1]
every = int(sys.argv[2]) if len(sys.argv) > 2 else 20
f_from = int(sys.argv[3]) if len(sys.argv) > 3 else 0
label = sys.argv[4] if len(sys.argv) > 4 else log
pat = re.compile(r'JAMCAM frame=(\d+) .*?px=(\S+) py=(\S+) .*?fyaw=(\S+) fpitch=(\S+) fcx=(\S+) fcy=(\S+) fh=(\S+) pkx=(\S+) pky=(\S+) hyaw=(\S+) bw=(\S+) rl=(\S+) rw=(\S+) rr=(\S+)')
rows = {}
for line in open(log, encoding='utf-8', errors='replace'):
    m = pat.search(line)
    if not m: continue
    v = [float(x) for x in m.groups()]
    rows[int(v[0])] = v
frames = sorted(rows)
def board(px, py, L, W):
    return min(L / 2 - px, L / 2 + px, W / 2 - py, W / 2 + py)
def count(sel, name):
    n = len(sel)
    if not n: print(f'{label} {name}: no frames'); return
    c = dict(ice55=0, ice40=0, steep55=0, steep60=0, him=0, chest=0, ahead=0, puckn=0, puck=0, out=0)
    mn = 1.0; sm = 0.0; mx = 0.0
    for f in sel:
        _, px, py, yaw, pitch, cx, cy, ch, pkx, pky, hyaw, bw, L, W, R = rows[f]
        cam = (cx, cy, ch)
        ice = ice_share(cam, yaw, pitch, L, W, R, cols=32, rows=18)
        mn = min(mn, ice); sm += ice; mx = max(mx, -pitch)
        c['ice55'] += ice < 0.55; c['ice40'] += ice < 0.40; c['steep55'] += -pitch > 55.01; c['steep60'] += -pitch > 60
        on = lambda p: (lambda q: abs(q[0]) <= 1 and abs(q[1]) <= 1)(frac(cam, yaw, pitch, p))
        chest = on((px, py, 1.0)); him = chest or on((px, py, 1.8)) or on((px, py, 0.2))
        c['chest'] += not chest; c['him'] += not him
        p0 = rows.get(f - 1)
        if p0 and math.hypot(px - p0[1], py - p0[2]) > 0.5 / 60 and math.hypot(px - p0[1], py - p0[2]) < 1.0:
            dx, dy = px - p0[1], py - p0[2]; l = math.hypot(dx, dy)
            c['ahead'] += not on((px + 3 * dx / l, py + 3 * dy / l, 0.0))
        if math.hypot(pkx - px, pky - py) < 20:
            c['puckn'] += 1; c['puck'] += not on((pkx, pky, 0.05))
        c['out'] += not inside(cx, cy, L, W, R, 0.3)
    pct = lambda a, b: f'{100.0 * a / b:.1f} %' if b else '-'
    print(f'{label} {name}: n {n} | ice<55% {pct(c["ice55"], n)} | ice<40% {pct(c["ice40"], n)} | min {mn:.2f} mean {sm / n:.2f} | pitch>55 {pct(c["steep55"], n)} (>60 {pct(c["steep60"], n)}) max {mx:.1f}'
          f' | him off {pct(c["him"], n)} (chest off {pct(c["chest"], n)}) | 3 m ahead off {pct(c["ahead"], n)} | puck off {pct(c["puck"], c["puckn"])} of {c["puckn"]} | eye out {c["out"]}')
sel = [f for f in frames if f >= f_from and (f - f_from) % every == 0]
count(sel, f'every {every}th frame')
near = [f for f in sel if board(rows[f][1], rows[f][2], rows[f][12], rows[f][13]) < 5.0]
count(near, 'of those, within 5 m of a board')
# the camera's own turn: over 30 frames, its yaw change beyond his own
flips = 0; worst = 0.0
for f in frames:
    a = rows.get(f - 30)
    if not a: continue
    dy = abs((rows[f][3] - a[3] + 180) % 360 - 180); dh = abs((rows[f][10] - a[10] + 180) % 360 - 180)
    worst = max(worst, dy - dh)
    if dy > 90 and dy > dh + 10: flips += 1
print(f'{label} flips (camera yaw > 90 deg in 30 frames beyond his own): {flips}; worst own turn {worst:.1f} deg; frames {len(frames)}')
