# Compose rendered frames into one small strip PNG (PIL). Keeps the result under a byte cap by shrinking.
#   python -I tools/meshy/strip.py <frames_dir> <out.png> [cols] [max_kb] [label]
import sys, os, glob
from PIL import Image, ImageDraw

src, out = sys.argv[1], sys.argv[2]
cols = int(sys.argv[3]) if len(sys.argv) > 3 else 8
max_kb = int(sys.argv[4]) if len(sys.argv) > 4 else 280
label = sys.argv[5] if len(sys.argv) > 5 else ""
frames = [Image.open(p).convert("RGB") for p in sorted(glob.glob(os.path.join(src, "f*.png")))]
w, h = frames[0].size
rows = (len(frames) + cols - 1) // cols
top = 18 if label else 0
img = Image.new("RGB", (w * cols, h * rows + top), (255, 255, 255))
for i, f in enumerate(frames):
    img.paste(f, ((i % cols) * w, (i // cols) * h + top))
if label:
    ImageDraw.Draw(img).text((6, 3), label, fill=(20, 20, 30))
scale = 1.0
while True:
    im = img if scale == 1.0 else img.resize((int(img.width * scale), int(img.height * scale)), Image.LANCZOS)
    im = im.quantize(colors=128, method=Image.Quantize.MEDIANCUT).convert("P")
    im.save(out, optimize=True)
    kb = os.path.getsize(out) / 1024
    if kb <= max_kb or scale < 0.3:
        break
    scale *= 0.85
print(out, im.size, round(kb, 1), "KB")
