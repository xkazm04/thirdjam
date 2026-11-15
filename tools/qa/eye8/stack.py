# Lane A230: stacks sheets under one another (the widest sets the width), with an optional title bar per sheet.
#   python stack.py <out.jpg> <in1.jpg[=title]> <in2.jpg[=title]> ...
import sys
from PIL import Image, ImageDraw

ims = []
for a in sys.argv[2:]:
    path, _, title = a.partition('=')
    ims.append((Image.open(path).convert('RGB'), title))
w = max(i.width for i, _ in ims)
h = sum(i.height + (18 if t else 0) for i, t in ims)
out = Image.new('RGB', (w, h), (16, 16, 16))
y = 0
for im, t in ims:
    if t:
        ImageDraw.Draw(out).text((6, y + 3), t, fill=(255, 255, 255))
        y += 18
    out.paste(im, (0, y))
    y += im.height
out.save(sys.argv[1], quality=88)
print(sys.argv[1], out.size)
