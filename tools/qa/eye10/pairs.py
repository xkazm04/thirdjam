# Lane A235: a before / after pair of the same sim frames of two films of the same seeded match (editor binary of this worktree).
# Top row BEFORE (the variable at its neutral value), bottom row AFTER (the default), the same frames left to right, each cropped to the same box.
#   python -I tools/qa/eye10/pairs.py <before run> <after run> <frames a,b,c> <crop x0,y0,x1,y1> <out.jpg> "<before label>" "<after label>" "<caption>"
import sys, os
from PIL import Image, ImageDraw, ImageFont
here = os.path.dirname(os.path.abspath(__file__))
def font(n):
    try: return ImageFont.truetype('arialbd.ttf', n)
    except OSError: return ImageFont.load_default()
def shot(run, f):
    d = os.path.join(here, 'out', run, 'shots')
    for n in os.listdir(d):
        if n.endswith('-f%05d.png' % f): return Image.open(os.path.join(d, n)).convert('RGB')
    raise SystemExit('no still f%d in %s' % (f, run))
def main():
    b, a, frames, box, out, lb, la, cap = sys.argv[1:9]
    frames = [int(x) for x in frames.split(',')]
    box = tuple(int(x) for x in box.split(','))
    W = 1920 // len(frames)
    h = round((box[3] - box[1]) * W / (box[2] - box[0]))
    sheet = Image.new('RGB', (W * len(frames), 40 + 2 * (h + 30)), (14, 16, 20))
    d = ImageDraw.Draw(sheet)
    d.text((10, 8), cap + '   (editor binary of the lane\'s tree, the same seeded match, the same sim frames)', fill=(240, 240, 240), font=font(20))
    for r, (run, lab) in enumerate(((b, lb), (a, la))):
        y = 40 + r * (h + 30)
        d.text((10, y + 4), lab, fill=(255, 210, 90) if r == 0 else (120, 230, 140), font=font(18))
        for i, f in enumerate(frames):
            im = shot(run, f).crop(box).resize((W, h), Image.LANCZOS)
            sheet.paste(im, (i * W, y + 30))
            d.text((i * W + 6, y + 34), 'f%d' % f, fill=(255, 255, 0), font=font(18))
    sheet.save(out, quality=86)
    print('wrote', out, sheet.size)
main()
