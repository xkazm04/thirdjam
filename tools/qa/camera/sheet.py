# Lane A211 (the eye): a contact sheet of a run's stills by sim frame.
#   python sheet.py <shots dir> <out.jpg> <from> <to> <step> [cols=4] [w=640] [crop=x0,y0,x1,y1] [prefix=u1-]
# Takes every still u1-<tier>-f<frame>.png (or any name ending -f<frame>.png with the prefix) whose frame is in [from, to] and on the step from <from>,
# resizes to w wide (the height follows), labels each with its frame, and pastes them in rows. Keep w large enough that a skater at play distance
# (about 30 px tall at 1280 wide) is still 15 px: 640 at the least for the whole frame; use crop for a closer look.
import sys, os, re
from PIL import Image, ImageDraw
d, out, f0, f1, step = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5])
cols, w, crop, prefix = 4, 640, None, 'u1-'
for a in sys.argv[6:]:
    k, v = a.split('=', 1)
    if k == 'cols': cols = int(v)
    elif k == 'w': w = int(v)
    elif k == 'crop': crop = tuple(int(x) for x in v.split(','))
    elif k == 'prefix': prefix = v
files = {}
for n in os.listdir(d):
    m = re.match(r'^' + re.escape(prefix) + r'.*-f0*(\d+)\.png$', n)
    if m: files[int(m.group(1))] = os.path.join(d, n)
pick = [f for f in sorted(files) if f0 <= f <= f1 and (f - f0) % step == 0]
if not pick:
    print('no stills in range', f0, f1, 'have', len(files)); sys.exit(1)
ims = []
for f in pick:
    try: im = Image.open(files[f]).convert('RGB')
    except Exception as e:
        print('bad', files[f]); continue
    if crop: im = im.crop(crop)
    h = round(im.height * w / im.width)
    im = im.resize((w, h), Image.LANCZOS)
    dr = ImageDraw.Draw(im)
    lab = 'f%d' % f
    dr.rectangle((0, 0, 8 + 7 * len(lab), 14), fill=(0, 0, 0))
    dr.text((3, 2), lab, fill=(255, 255, 0))
    ims.append(im)
h = ims[0].height
rows = (len(ims) + cols - 1) // cols
S = Image.new('RGB', (w * cols, h * rows), (20, 20, 20))
for i, im in enumerate(ims): S.paste(im, ((i % cols) * w, (i // cols) * h))
S.save(out, quality=90)
print(out, len(ims), 'stills', S.size)
