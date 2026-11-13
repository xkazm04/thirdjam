# Lane A123 (track U1): contact sheets of a stills run, with the frame's middle third drawn in, for reading 30 stills a tier by eye.
#   python tools/qa/screen/u1-sheet.py tools/qa/screen/out/clay-after            -> out/clay-after/sheet-1.jpg ... (9 stills a sheet, 640 x 360 each)
#   python tools/qa/screen/u1-sheet.py <run dir> --pair <other run dir> <frame>  -> pair-<frame>.jpg: the same sim frame of two runs side by side
#   python tools/qa/screen/u1-sheet.py <run dir> --crop <frame> x y w h [scale]  -> crop-<frame>.jpg: a part of one still, enlarged
# The middle third is the centre rectangle [W/3, 2W/3] x [H/3, 2H/3]; the sheets outline it in thin magenta (the single stills carry no outline).
import glob, os, re, sys
from PIL import Image, ImageDraw

def stills(run):
    files = glob.glob(os.path.join(run, 'shots', '*.png'))
    def key(p):
        m = re.search(r'-f(\d+)\.png$', p)
        return int(m.group(1)) if m else -1
    return sorted(files, key=key)

def frame_of(p):
    m = re.search(r'-f(\d+)\.png$', p)
    return ('f' + m.group(1)) if m else 'first'

def find(run, frame):
    for p in stills(run):
        if frame_of(p) == frame or frame_of(p) == 'f' + str(frame).zfill(5):
            return p
    raise SystemExit('no still for frame %s in %s' % (frame, run))

def sheet(run):
    files = stills(run)
    tw, th, cols, rows = 640, 360, 3, 3
    n = 0
    for i in range(0, len(files), cols * rows):
        n += 1
        im = Image.new('RGB', (tw * cols, th * rows), (0, 0, 0))
        for j, p in enumerate(files[i:i + cols * rows]):
            t = Image.open(p).convert('RGB').resize((tw, th), Image.LANCZOS)
            d = ImageDraw.Draw(t)
            d.rectangle([tw / 3, th / 3, tw * 2 / 3, th * 2 / 3], outline=(255, 0, 255), width=1)
            d.rectangle([0, 0, 74, 14], fill=(0, 0, 0))
            d.text((3, 2), frame_of(p), fill=(255, 255, 255))
            im.paste(t, ((j % cols) * tw, (j // cols) * th))
        out = os.path.join(run, 'sheet-%d.jpg' % n)
        im.save(out, quality=88)
        print(out)

def pair(run_a, run_b, frame, out=None):
    a, b = Image.open(find(run_a, frame)).convert('RGB'), Image.open(find(run_b, frame)).convert('RGB')
    im = Image.new('RGB', (a.width + b.width + 8, max(a.height, b.height)), (0, 0, 0))
    im.paste(a, (0, 0)); im.paste(b, (a.width + 8, 0))
    out = out or os.path.join(run_a, 'pair-%s.jpg' % frame)
    im.save(out, quality=90)
    print(out)

def crop(run, frame, x, y, w, h, scale=2.0, out=None):
    a = Image.open(find(run, frame)).convert('RGB').crop((x, y, x + w, y + h))
    a = a.resize((int(w * scale), int(h * scale)), Image.LANCZOS)
    out = out or os.path.join(run, 'crop-%s.jpg' % frame)
    a.save(out, quality=92)
    print(out)

if __name__ == '__main__':
    a = sys.argv[1:]
    if not a:
        raise SystemExit(__doc__ or 'usage: u1-sheet.py <run dir> [--pair <run dir> <frame> | --crop <frame> x y w h [scale]]')
    if len(a) >= 4 and a[1] == '--pair':
        pair(a[0], a[2], a[3], a[4] if len(a) > 4 else None)
    elif len(a) >= 7 and a[1] == '--crop':
        crop(a[0], a[2], int(a[3]), int(a[4]), int(a[5]), int(a[6]), float(a[7]) if len(a) > 7 else 2.0, a[8] if len(a) > 8 else None)
    else:
        sheet(a[0])
