# Lane A223: sheets of a look's stills (eye3-run.ps1 -Every N: u1-<tier>-f<frame>.png).
#   python looks.py sheet <run dir> <out.jpg> <frames: 300,900,1800 or from:to:step> [w=640] [cols=3] [title=...] [crop=x0,y0,x1,y1 in 0..1]
#   python looks.py ab <before run dir> <after run dir> <out.jpg> <frames> [w=640] [cols=3] [title=...] [bl=BEFORE] [al=AFTER] [crop=...]
#   python looks.py lum <run dir> <frames> [crop=...]     mean and 99th percentile luminance (0..255) of each still (a number beside the eye, never instead of it)
# ab: each cell is one sim frame, the before still above the after still. A frame missing from a run is a dark cell marked "no still".
import sys, os, re
from PIL import Image, ImageDraw


def index(d):
    d = os.path.join(d, 'shots') if os.path.isdir(os.path.join(d, 'shots')) else d
    out = {}
    for n in os.listdir(d):
        m = re.match(r'^u1-[a-z0-9_]+-f0*(\d+)\.png$', n)
        if m: out[int(m.group(1))] = os.path.join(d, n)
    return out


def frames(spec, have):
    if ':' in spec:
        a, b, s = (int(x) for x in spec.split(':'))
        return [f for f in range(a, b + 1, s)]
    return [int(x) for x in spec.split(',')]


def opts(args):
    o = {'w': 640, 'cols': 3, 'title': '', 'bl': 'BEFORE', 'al': 'AFTER', 'crop': None}
    for a in args:
        k, v = a.split('=', 1)
        o[k] = int(v) if k in ('w', 'cols') else (tuple(float(x) for x in v.split(',')) if k == 'crop' else v)
    return o


def load(path, o):
    im = Image.open(path).convert('RGB')
    if o['crop']:
        c = o['crop']
        im = im.crop((round(c[0] * im.width), round(c[1] * im.height), round(c[2] * im.width), round(c[3] * im.height)))
    return im


def cell(path, f, lab, colour, o, ch):
    w = o['w']
    if path is None:
        im = Image.new('RGB', (w, ch), (30, 30, 30))
        ImageDraw.Draw(im).text((8, ch // 2), 'no still', fill=(200, 200, 200))
    else:
        im = load(path, o).resize((w, ch), Image.LANCZOS)
    t = ('%s f%d' % (lab, f)).strip()
    dr = ImageDraw.Draw(im)
    dr.rectangle((0, 0, 8 + 6 * len(t), 13), fill=(0, 0, 0))
    dr.text((3, 1), t, fill=colour)
    return im


def height(o, sample):
    if sample is None: return round(o['w'] * 9 / 16)
    im = load(sample, o)
    return round(o['w'] * im.height / im.width)


mode = sys.argv[1]
if mode == 'lum':
    I = index(sys.argv[2]); o = opts(sys.argv[4:])
    for f in frames(sys.argv[3], I):
        if f not in I: print(f, 'no still'); continue
        g = load(I[f], o).convert('L'); h = g.histogram(); n = sum(h); acc = 0; p99 = 255
        for v in range(256):
            acc += h[v]
            if acc >= 0.99 * n: p99 = v; break
        print('f%d mean %.1f p99 %d' % (f, sum(v * c for v, c in enumerate(h)) / n, p99))
    sys.exit(0)
if mode == 'sheet':
    I = index(sys.argv[2]); out = sys.argv[3]; pick = frames(sys.argv[4], I); o = opts(sys.argv[5:]); rowsper = 1
    runs = [(I, '', (255, 255, 255))]
else:
    B, A = index(sys.argv[2]), index(sys.argv[3]); out = sys.argv[4]; pick = frames(sys.argv[5], B); o = opts(sys.argv[6:]); rowsper = 2
    runs = [(B, o['bl'], (255, 220, 0)), (A, o['al'], (120, 255, 120))]
sample = next((r[0][f] for r in runs for f in pick if f in r[0]), None)
ch = height(o, sample); w, cols, gap = o['w'], o['cols'], 6
head = 22 if o['title'] else 0
rows = (len(pick) + cols - 1) // cols
S = Image.new('RGB', (cols * w + (cols - 1) * gap, head + rows * (rowsper * ch + gap)), (16, 16, 16))
if o['title']: ImageDraw.Draw(S).text((6, 5), o['title'], fill=(255, 255, 255))
for i, f in enumerate(pick):
    x, y = (i % cols) * (w + gap), head + (i // cols) * (rowsper * ch + gap)
    for k, (I, lab, col) in enumerate(runs):
        S.paste(cell(I.get(f), f, lab, col, o, ch), (x, y + k * ch))
os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
S.save(out, quality=86)
print(out, len(pick), 'frames', S.size)
