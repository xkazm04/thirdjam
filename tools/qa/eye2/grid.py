import sys
from PIL import Image, ImageDraw
# grid.py out.jpg cols W H [crop=x0,y0,x1,y1] "label|file" ... : stills side by side, each resized to W x H (after an optional crop), a label on each (lane A210)
out, cols, w, h = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])
rest = sys.argv[5:]
crop = None
if rest and rest[0].startswith('crop='):
    crop = tuple(int(x) for x in rest[0][5:].split(','))
    rest = rest[1:]
cells = [r.split('|', 1) for r in rest]
rows = (len(cells) + cols - 1) // cols
S = Image.new('RGB', (w * cols, h * rows), (20, 20, 20))
for i, (label, f) in enumerate(cells):
    try: im = Image.open(f).convert('RGB')
    except Exception as e:
        print('missing', f); continue
    if crop: im = im.crop(crop)
    im = im.resize((w, h), Image.LANCZOS)
    d = ImageDraw.Draw(im)
    d.rectangle((0, 0, 8 + 6 * len(label), 14), fill=(0, 0, 0))
    d.text((3, 2), label, fill=(255, 255, 0))
    S.paste(im, ((i % cols) * w, (i // cols) * h))
S.save(out, quality=88)
print(out, len(cells))
