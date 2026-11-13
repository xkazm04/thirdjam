# Lane A77: a labelled strip out of the game's own stills (jam.ctrl.Stills 2 writes a77-<kind>-<start>-f<frame>.png every 4 sim frames). A73's tool with the prefix
# (STRIP_PREFIX, default a77) and a tile stride (STRIP_STRIDE: 1 = every still, 4 = every fourth) from the environment.
#   python -I tools/qa/ctrl/a77-strip.py <stills dir> <kind>-<start> <out.png> [cols] [tile width px] [caption] [max KB]
# Each tile carries its sim frame and the offset from the start frame; the whole strip is kept under about 600 KB.
import sys, os, glob, re
from PIL import Image, ImageDraw

src, key, out = sys.argv[1], sys.argv[2], sys.argv[3]
prefix = os.environ.get("STRIP_PREFIX", "a77")
stride = int(os.environ.get("STRIP_STRIDE", "1"))
cols = int(sys.argv[4]) if len(sys.argv) > 4 else 6
tile_w = int(sys.argv[5]) if len(sys.argv) > 5 else 400
caption = sys.argv[6] if len(sys.argv) > 6 else ""
max_kb = int(sys.argv[7]) if len(sys.argv) > 7 else 600
files = sorted(glob.glob(os.path.join(src, f"{prefix}-{key}-f*.png")), key=lambda p: int(re.search(r"-f(\d+)\.png$", p).group(1)))
if not files:
    print("no stills for", key); sys.exit(2)
files = files[::stride]
start = int(key.rsplit("-", 1)[1])
tiles = []
for p in files:
    f = int(re.search(r"-f(\d+)\.png$", p).group(1))
    im = Image.open(p).convert("RGB")
    im = im.resize((tile_w, int(im.height * tile_w / im.width)), Image.LANCZOS)
    d = ImageDraw.Draw(im)
    label = f"f{f}  +{f - start}"
    d.rectangle((0, 0, 8 + 7 * len(label), 16), fill=(10, 12, 20))
    d.text((4, 2), label, fill=(240, 240, 240))
    tiles.append(im)
w, h = tiles[0].size
rows = (len(tiles) + cols - 1) // cols
top = 18 if caption else 0
img = Image.new("RGB", (w * cols, h * rows + top), (24, 26, 34))
for i, t in enumerate(tiles):
    img.paste(t, ((i % cols) * w, (i // cols) * h + top))
if caption:
    ImageDraw.Draw(img).text((6, 3), caption, fill=(235, 235, 235))
os.makedirs(os.path.dirname(out) or ".", exist_ok=True)
scale = 1.0
while True:
    im = img if scale == 1.0 else img.resize((int(img.width * scale), int(img.height * scale)), Image.LANCZOS)
    im.save(out, optimize=True)
    kb = os.path.getsize(out) / 1024
    if kb <= max_kb or scale < 0.4:
        break
    scale *= 0.85
print(out, im.size, round(kb, 1), "KB", len(tiles), "tiles")
