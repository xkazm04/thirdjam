# Lane A226: a contact sheet of named stills.
#   python contact.py <out.jpg> <cols> <cell width> <label=path> [<label=path> ...]
import sys, os
from PIL import Image, ImageDraw, ImageFont
out, cols, w = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
try: font = ImageFont.truetype('arial.ttf', 18)
except Exception: font = ImageFont.load_default()
cells = []
for a in sys.argv[4:]:
    lab, path = a.split('=', 1)
    im = Image.open(path).convert('RGB')
    im = im.resize((w, round(w * im.height / im.width)), Image.LANCZOS)
    c = Image.new('RGB', (w, im.height + 26), (16, 16, 16)); c.paste(im, (0, 26))
    ImageDraw.Draw(c).text((6, 3), lab, fill=(255, 255, 255), font=font)
    cells.append(c)
h = max(c.height for c in cells); gap = 8; rows = (len(cells) + cols - 1) // cols
S = Image.new('RGB', (cols * w + (cols + 1) * gap, rows * h + (rows + 1) * gap), (16, 16, 16))
for i, c in enumerate(cells): S.paste(c, (gap + (i % cols) * (w + gap), gap + (i // cols) * (h + gap)))
os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
S.save(out, quality=90); print(out, S.size, len(cells))
