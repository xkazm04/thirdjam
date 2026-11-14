import sys, glob
from PIL import Image
# sheet.py <shots dir> <prefix> <out.jpg> <offsets comma list> [cols]
d, prefix, out, offs = sys.argv[1], sys.argv[2], sys.argv[3], [int(x) for x in sys.argv[4].split(',')]
cols = int(sys.argv[5]) if len(sys.argv) > 5 else 4
fs = sorted(glob.glob(f'{d}/{prefix}-f*.png'))
f0 = int(fs[0][-10:-4])
pick = []
for o in offs:
    m = [f for f in fs if int(f[-10:-4]) == f0 + o]
    if m: pick.append((o, m[0]))
w, h = 480, 270
rows = (len(pick) + cols - 1) // cols
S = Image.new('RGB', (w * cols, h * rows))
from PIL import ImageDraw
for i, (o, f) in enumerate(pick):
    im = Image.open(f).convert('RGB').resize((w, h))
    ImageDraw.Draw(im).text((6, 4), f'+{o}  f{int(f[-10:-4])}', fill=(255, 255, 0))
    S.paste(im, ((i % cols) * w, (i // cols) * h))
S.save(out, quality=85)
print(out, len(pick), 'of', len(fs))
