# Lane A223: the final before / after sheet of one look: the same sim frames at 1920 x 1080 (upper block) and 1280 x 720 (lower block), the neutral
# values above the defaults in each cell.   python final.py <before 1080> <after 1080> <before 720> <after 720> <out.jpg> <frames> "<title>"
import sys, subprocess, os
from PIL import Image
b1, a1, b7, a7, out, frames, title = sys.argv[1:8]
here = os.path.dirname(os.path.abspath(__file__))
tmp = [out + '.1080.tmp.jpg', out + '.720.tmp.jpg']
for (b, a, t, size) in ((b1, a1, tmp[0], '1920 x 1080'), (b7, a7, tmp[1], '1280 x 720')):
    subprocess.check_call([sys.executable, os.path.join(here, 'looks.py'), 'ab', b, a, t, frames, 'w=800', 'cols=4', 'title=%s, %s' % (title, size)])
ims = [Image.open(t) for t in tmp]
S = Image.new('RGB', (max(i.width for i in ims), sum(i.height for i in ims) + 8), (16, 16, 16))
y = 0
for i in ims:
    S.paste(i, (0, y)); y += i.height + 8
S.save(out, quality=85)
for t in tmp: os.remove(t)
print(out, S.size)
