# Lane A48: a labelled contact sheet of a capture run, to pick the storyboard stills by eye.
#   python tools/qa/storyboard/sheet.py <shots dir> <out.png> [--cols 6] [--width 320] [--from t000s] [--to t999s]
import sys, os, argparse
from PIL import Image, ImageDraw

ap = argparse.ArgumentParser()
ap.add_argument('shots'); ap.add_argument('out')
ap.add_argument('--cols', type=int, default=6); ap.add_argument('--width', type=int, default=320)
ap.add_argument('--from', dest='lo', default=''); ap.add_argument('--to', dest='hi', default='zzzz')
a = ap.parse_args()
names = sorted(n for n in os.listdir(a.shots) if n.endswith('.png') and a.lo <= n[:-4] <= a.hi)
if not names: sys.exit('no stills')
w = a.width; h = int(w * 9 / 16); pad = 18
rows = (len(names) + a.cols - 1) // a.cols
sheet = Image.new('RGB', (a.cols * w, rows * (h + pad)), (20, 20, 20))
d = ImageDraw.Draw(sheet)
for i, n in enumerate(names):
    im = Image.open(os.path.join(a.shots, n)).convert('RGB').resize((w, h), Image.LANCZOS)
    x = (i % a.cols) * w; y = (i // a.cols) * (h + pad)
    sheet.paste(im, (x, y + pad))
    d.text((x + 4, y + 3), n[:-4], fill=(255, 230, 120))
sheet.save(a.out)
print('PASS sheet', a.out, len(names), 'stills')
