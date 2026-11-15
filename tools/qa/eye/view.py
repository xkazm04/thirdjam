# Lane A211: what the game camera shows, still by still: the share of the picture that is ice (the pale lavender of the p3_clay rink).
#   python view.py <shots dir> [from] [to] [step]
# A still with less than 45 % ice is mostly boards and crowd ("stands"); one whose top band (where the crowd is in the open-ice view) is ice is the
# look from above with no horizon ("top"). Calibrated by eye on run r1-rookie (frames 500 to 4300, every 40th: 42 by eye, 40 + 13 by this tool, which
# also counts the three goal-banner stills and the steep looks the eye let pass).
import sys, os, re
from PIL import Image
d = sys.argv[1]
f0 = int(sys.argv[2]) if len(sys.argv) > 2 else 0
f1 = int(sys.argv[3]) if len(sys.argv) > 3 else 10**9
step = int(sys.argv[4]) if len(sys.argv) > 4 else 20
rows = []
for n in sorted(os.listdir(d)):
    m = re.match(r'^u1-.*-f0*(\d+)\.png$', n)
    if not m: continue
    f = int(m.group(1))
    if f < f0 or f > f1 or (f - f0) % step: continue
    im = Image.open(os.path.join(d, n)).convert('RGB').resize((160, 90))
    px = list(im.getdata())
    ice = 0; top = 0
    for i, (r, g, b) in enumerate(px):
        isice = r > 125 and g > 110 and b > 115 and (r - g) < 34 and b > g - 6 and b > 0.82 * r   # pale lavender-grey; wood is r >> b
        ice += isice
        if i < 160 * 14: top += isice
    rows.append((f, ice / len(px), top / (160 * 14)))
st = [r for r in rows if r[1] < 0.45]
tp = [r for r in rows if r[1] >= 0.45 and r[2] > 0.4]   # no horizon: the top band of the picture is ice
print(d, 'stills', len(rows), 'stands', len(st), 'top-down', len(tp), 'share', round((len(st) + len(tp)) / max(1, len(rows)), 2))
if '-v' in sys.argv:
    print(' stands:', [r[0] for r in st]); print(' top:', [r[0] for r in tp])
