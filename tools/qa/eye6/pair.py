# Lane A219: a before / after contact sheet of the SAME sim frames from two runs (the game's hit strips or the every-frame stills).
#   python pair.py <before dir> <after dir> <out.jpg> <from> <to> <step> [cols=6] [w=320] [crop=x0,y0,x1,y1] [prefix=a196-cross-000391] [title=...]
#   [bl=BEFORE] [al=AFTER]
# Each cell is one frame: the before still above the after still, the frame number on both. crop is given for a 1280 x 720 still and is scaled to the
# stills' own size (a 1920 x 1080 run takes the same crop). A frame missing from one run is drawn as a dark cell marked "no still".
import sys, os, re
from PIL import Image, ImageDraw
bd, ad, out, f0, f1, step = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4]), int(sys.argv[5]), int(sys.argv[6])
cols, w, crop, prefix, title, bl, al = 6, 320, None, 'a196-', '', 'BEFORE', 'AFTER'
for a in sys.argv[7:]:
    k, v = a.split('=', 1)
    if k == 'cols': cols = int(v)
    elif k == 'w': w = int(v)
    elif k == 'crop': crop = tuple(int(x) for x in v.split(','))
    elif k == 'prefix': prefix = v
    elif k == 'title': title = v
    elif k == 'bl': bl = v
    elif k == 'al': al = v


def index(d):
    files = {}
    for n in os.listdir(d):
        m = re.match(r'^' + re.escape(prefix) + r'.*-f0*(\d+)\.png$', n)
        if m: files[int(m.group(1))] = os.path.join(d, n)
    return files


def cell(path, f, lab, colour):
    if path is None:
        im = Image.new('RGB', (w, ch), (30, 30, 30))
        ImageDraw.Draw(im).text((8, ch // 2), 'no still', fill=(200, 200, 200))
    else:
        im = Image.open(path).convert('RGB')
        if crop:
            k = im.width / 1280.0
            im = im.crop(tuple(round(c * k) for c in crop))
        im = im.resize((w, ch), Image.LANCZOS)
    dr = ImageDraw.Draw(im)
    t = '%s f%d' % (lab, f)
    dr.rectangle((0, 0, 8 + 6 * len(t), 13), fill=(0, 0, 0))
    dr.text((3, 1), t, fill=colour)
    return im


B, A = index(bd), index(ad)
pick = [f for f in sorted(set(B) | set(A)) if f0 <= f <= f1 and (f - f0) % step == 0]
if not pick:
    print('no stills in range', f0, f1, 'before', len(B), 'after', len(A)); sys.exit(1)
ch = round(w * ((crop[3] - crop[1]) / (crop[2] - crop[0]) if crop else 9 / 16))
gap, head = 6, 22 if title else 0
rows = (len(pick) + cols - 1) // cols
S = Image.new('RGB', (cols * w + (cols - 1) * gap, head + rows * (2 * ch + gap)), (16, 16, 16))
if title: ImageDraw.Draw(S).text((6, 5), title, fill=(255, 255, 255))
for i, f in enumerate(pick):
    x, y = (i % cols) * (w + gap), head + (i // cols) * (2 * ch + gap)
    S.paste(cell(B.get(f), f, bl, (255, 220, 0)), (x, y))
    S.paste(cell(A.get(f), f, al, (120, 255, 120)), (x, y + ch))
S.save(out, quality=84)
print(out, len(pick), 'frames', S.size, 'missing before', sum(1 for f in pick if f not in B), 'after', sum(1 for f in pick if f not in A))
