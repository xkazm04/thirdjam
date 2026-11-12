# tools/p4/sheet.py (lane A28): contact sheet + clip from a capture folder (PIL + ffmpeg).
#   python -I tools/p4/sheet.py <cap_dir> <tag> <cam chase|far> <out.png> [out.mp4] [label]
# Tiles <tag>_f00.._f15_<cam>.png (the 2 s clip, one frame per 8 game frames) on row 1-2, rag00..05 + back00..03 on row 3 (the knocked_down
# ragdoll window and the return to the sequence). Cells are 320 x 240 with the frame name drawn in the corner.
import sys, os, glob, subprocess
from PIL import Image, ImageDraw
cap, tag, cam, out = sys.argv[1:5]
mp4 = sys.argv[5] if len(sys.argv) > 5 and sys.argv[5] != "-" else None
label = sys.argv[6] if len(sys.argv) > 6 else ""
def frames(prefix):
    return sorted(glob.glob(os.path.join(cap, "%s_%s*_%s.png" % (tag, prefix, cam))))
rows = [frames("f")[:8], frames("f")[8:16], frames("rag") + frames("back")]
rows = [r for r in rows if r]
if not rows:
    print("A28 sheet no frames in", cap, tag, cam); sys.exit(1)
W, H = 320, 240
cols = max(len(r) for r in rows)
top = 20 if label else 0
img = Image.new("RGB", (W * cols, H * len(rows) + top), (24, 24, 28))
d = ImageDraw.Draw(img)
if label:
    d.text((6, 4), label, fill=(230, 230, 230))
for ri, r in enumerate(rows):
    for ci, p in enumerate(r):
        im = Image.open(p).convert("RGB").resize((W, H), Image.LANCZOS)
        img.paste(im, (ci * W, top + ri * H))
        name = os.path.basename(p).replace(tag + "_", "").replace("_" + cam + ".png", "")
        d.text((ci * W + 6, top + ri * H + 4), name, fill=(255, 240, 120))
img.save(out, optimize=True)
print("A28 sheet", out, img.size, round(os.path.getsize(out) / 1024), "KB")
if mp4:
    seq = frames("f")
    tmp = os.path.join(cap, "_seq_" + cam)
    os.makedirs(tmp, exist_ok=True)
    for i, p in enumerate(seq + frames("rag") + frames("back")):
        Image.open(p).convert("RGB").save(os.path.join(tmp, "s%03d.png" % i))
    r = subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-framerate", "7.5", "-i", os.path.join(tmp, "s%03d.png"), "-c:v", "libx264", "-pix_fmt", "yuv420p", "-crf", "24", mp4])
    print("A28 clip", mp4, os.path.getsize(mp4) if os.path.exists(mp4) else "FAILED", "rc", r.returncode)
