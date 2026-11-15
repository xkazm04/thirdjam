# Lane A230: one sheet of chosen stills from several runs.
#   python pick.py <out.jpg> <w> <cols> <run dir>:<frame>:<label> ...      (a run dir under out\, a frame number of that film)
import sys, os, re
from PIL import Image, ImageDraw

out, w, cols = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
cells = []
for a in sys.argv[4:]:
    run, frame, label = a.split(':', 2)
    d = os.path.join('out', run, 'shots')
    path = None
    for n in os.listdir(d):
        m = re.match(r'^u1-[a-z0-9_]+-f0*(\d+)\.png$', n)
        if m and int(m.group(1)) == int(frame): path = os.path.join(d, n)
    cells.append((path, label + ' f' + frame))
ch = round(w * 9 / 16)
rows = (len(cells) + cols - 1) // cols
S = Image.new('RGB', (cols * w + (cols - 1) * 6, rows * (ch + 6)), (16, 16, 16))
for i, (path, label) in enumerate(cells):
    x, y = (i % cols) * (w + 6), (i // cols) * (ch + 6)
    if path:
        im = Image.open(path).convert('RGB').resize((w, ch), Image.LANCZOS)
    else:
        im = Image.new('RGB', (w, ch), (30, 30, 30)); label += ' (no still)'
    dr = ImageDraw.Draw(im)
    dr.rectangle((0, 0, 8 + 6 * len(label), 13), fill=(0, 0, 0))
    dr.text((3, 1), label, fill=(255, 255, 255))
    S.paste(im, (x, y))
S.save(out, quality=88)
print(out, S.size)
