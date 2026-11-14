import sys, glob
from PIL import Image, ImageDraw
# pair.py out.jpg "label|shotsdir|prefix|offsets" ... : one row a strip (before above, after below)
out = sys.argv[1]
rows = [r.split('|') for r in sys.argv[2:]]
w, h = 320, 180
cols = max(len(r[3].split(',')) for r in rows)
S = Image.new('RGB', (w * cols, h * len(rows)), (20, 20, 20))
for ri, (label, d, prefix, offs) in enumerate(rows):
    fs = sorted(glob.glob(f'{d}/{prefix}-f*.png'))
    f0 = int(fs[0][-10:-4])
    for ci, o in enumerate(int(x) for x in offs.split(',')):
        m = [f for f in fs if int(f[-10:-4]) == f0 + o]
        if not m: continue
        im = Image.open(m[0]).convert('RGB').resize((w, h))
        dr = ImageDraw.Draw(im)
        dr.rectangle((0, 0, 150, 14), fill=(0, 0, 0))
        dr.text((3, 2), f'{label} +{o}', fill=(255, 255, 0))
        S.paste(im, (ci * w, ri * h))
S.save(out, quality=88)
print(out)
