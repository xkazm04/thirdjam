"""Lane A40: turn the HighResShot PNGs of a cam40 run into the report's JPEGs and a 4 x 2 strip.
  python -I tools/qa/cam40tile.py <screenshot dir> <tag> <out dir> <out prefix> <strip frames comma list>
Stills named <tag>-fNNNNN.png become <out prefix>-fNNNNN.jpg (2560 wide, quality 82); the strip frames are tiled 4 x 2 at 1280 px each
into <out prefix>-strip.jpg with the sim frame burnt into each tile's corner.
"""
import os, sys
from PIL import Image, ImageDraw

src, tag, out, prefix, strip = sys.argv[1:6]
frames = [int(x) for x in strip.split(',') if x.strip()]
os.makedirs(out, exist_ok=True)
made = []
for f in sorted(os.listdir(src)):
    if f.startswith(tag + '-f') and f.endswith('.png'):
        im = Image.open(os.path.join(src, f)).convert('RGB')
        if im.width > 2560:
            im = im.resize((2560, round(im.height * 2560 / im.width)), Image.LANCZOS)
        name = prefix + '-' + f[len(tag) + 1:-4] + '.jpg'
        im.save(os.path.join(out, name), quality=82, optimize=True)
        made.append(name)
tiles = []
for fr in frames:
    cand = [f for f in os.listdir(src) if f.startswith('%s-f' % tag) and f.endswith('.png') and int(f[len(tag) + 2:-4]) >= fr]
    if not cand:
        continue
    cand.sort(key=lambda f: int(f[len(tag) + 2:-4]))
    f = cand[0]
    im = Image.open(os.path.join(src, f)).convert('RGB')
    im = im.resize((1280, round(im.height * 1280 / im.width)), Image.LANCZOS)
    d = ImageDraw.Draw(im)
    label = 'f%d' % int(f[len(tag) + 2:-4])
    d.rectangle([8, 8, 8 + 10 * len(label) + 12, 34], fill=(0, 0, 0))
    d.text((14, 12), label, fill=(255, 255, 255))
    tiles.append(im)
if tiles:
    w, h = tiles[0].size
    cols, rows = 4, (len(tiles) + 3) // 4
    sheet = Image.new('RGB', (cols * w, rows * h), (0, 0, 0))
    for i, t in enumerate(tiles):
        sheet.paste(t, ((i % cols) * w, (i // cols) * h))
    sheet.save(os.path.join(out, prefix + '-strip.jpg'), quality=80, optimize=True)
    made.append(prefix + '-strip.jpg')
print('\n'.join(made))
