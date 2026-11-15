# Lane A229: before / after pairs of the HUD stills. Each pair is the same moment of a run's script, BEFORE on top (jam.hud.OneLook 0 on the same
# editor binary: the neutral value) and AFTER under it (jam.hud.OneLook 1), each scaled to 1280 wide, a caption band over each. JPEG, quality 88.
#   python -I tools/qa/eye7/pair.py <before.png> <after.png> <out.jpg> "<caption>"
import sys
from PIL import Image, ImageDraw, ImageFont

def band(text, w):
    b = Image.new('RGB', (w, 36), (14, 16, 20))
    d = ImageDraw.Draw(b)
    try:
        f = ImageFont.truetype('arialbd.ttf', 20)
    except OSError:
        f = ImageFont.load_default()
    d.text((12, 7), text, fill=(240, 240, 240), font=f)
    return b

def main():
    before, after, out, cap = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
    W = 1280
    tiles = []
    for path, tag in ((before, 'BEFORE (jam.hud.OneLook 0)'), (after, 'AFTER (jam.hud.OneLook 1)')):
        im = Image.open(path).convert('RGB')
        src = '%dx%d' % im.size
        im = im.resize((W, round(im.size[1] * W / im.size[0])), Image.LANCZOS)
        tiles.append(band('%s   %s   (%s, editor binary)' % (tag, cap, src), W))
        tiles.append(im)
    H = sum(t.size[1] for t in tiles)
    sheet = Image.new('RGB', (W, H), (0, 0, 0))
    y = 0
    for t in tiles:
        sheet.paste(t, (0, y))
        y += t.size[1]
    sheet.save(out, quality=88)
    print('wrote', out, sheet.size)

if __name__ == '__main__':
    main()
