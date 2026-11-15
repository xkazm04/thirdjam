# Lane A226: the picked stills copied untouched to the delivery folder, a contact sheet and the picks list.
#   python deliver.py <out root> <run prefix> <tag> <dest dir> [check]
# A pick's optional "quote" (the booth's line in the frame) is written only for the first shoot (no run prefix): the line differs on every run.
# picks.json: [{ "n": 1, "what": "home-rookie-lit", "run": "m-front", "still": "fe-home" | "frame": 60, "caption": "...", "use": "page" | "more" }]
# A pick's file is <out root>/<run prefix><run>/shots/<still>.png or .../u1-<tier>-f<frame>.png. With "check" nothing is written: the picks are only looked up.
import sys, os, re, json, shutil
from PIL import Image, ImageDraw, ImageFont
here = os.path.dirname(os.path.abspath(__file__))
root, prefix, tag, dest = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
check = len(sys.argv) > 5 and sys.argv[5] == 'check'
picks = json.load(open(os.path.join(here, 'picks.json'), encoding='utf-8'))
def find(p):
    d = os.path.join(root, prefix + p['run'], 'shots')
    if 'still' in p:
        f = os.path.join(d, p['still'] + '.png'); return f if os.path.exists(f) else None
    for n in os.listdir(d) if os.path.isdir(d) else []:
        m = re.match(r'^u1-[a-z0-9_]+-f0*(\d+)\.png$', n)
        if m and int(m.group(1)) == p['frame']: return os.path.join(d, n)
    return None
bad = 0; rows = []
for p in picks:
    src = find(p)
    if not src: print('MISSING', p['n'], p['what'], p['run'], p.get('frame', p.get('still'))); bad += 1; continue
    im = Image.open(src)
    if im.size != (1920, 1080): print('SIZE', p['what'], im.size); bad += 1
    name = '%s-%02d-%s.png' % (tag, p['n'], p['what'])
    rows.append((p, src, name))
if bad or check: print('picks found:', len(rows), 'of', len(picks)); sys.exit(1 if bad else 0)
os.makedirs(dest, exist_ok=True)
for p, src, name in rows: shutil.copyfile(src, os.path.join(dest, name))
try: font = ImageFont.truetype('arialbd.ttf', 20)
except Exception: font = ImageFont.load_default()
w, cols, gap, head = 640, 4, 8, 28
rws = (len(rows) + cols - 1) // cols
S = Image.new('RGB', (cols * w + (cols + 1) * gap, rws * (360 + head) + (rws + 1) * gap), (16, 16, 16))
for i, (p, src, name) in enumerate(rows):
    x, y = gap + (i % cols) * (w + gap), gap + (i // cols) * (360 + head + gap)
    S.paste(Image.open(src).convert('RGB').resize((w, 360), Image.LANCZOS), (x, y + head))
    ImageDraw.Draw(S).text((x + 4, y + 3), name[:-4] + ('   [page]' if p.get('use') == 'page' else ''), fill=(255, 255, 255), font=font)
S.save(os.path.join(dest, '%s-contact.jpg' % tag), quality=90)
with open(os.path.join(dest, '%s-PICKS.md' % tag.upper()), 'w', encoding='utf-8', newline='\n') as f:
    intro = os.path.join(here, 'picks-intro.md')
    f.write(open(intro, encoding='utf-8').read().replace('{TAG}', tag.upper()).replace('{tag}', tag) if os.path.exists(intro) else '# %s picks\n\n' % tag.upper())
    f.write('| # | File | Use | Caption (true to the frame) | Run, frame |\n|---|---|---|---|---|\n')
    for p, src, name in rows:
        f.write('| %d | `%s` | %s | %s | `%s`, %s |\n' % (p['n'], name, 'the page\'s five' if p.get('use') == 'page' else 'gallery / cutaway', p['caption'] + ((' ' + p['quote']) if (p.get('quote') and not prefix) else ''), p['run'], ('frame %d' % p['frame']) if 'frame' in p else ('still `%s`' % p['still'])))
    if prefix: f.write("\nThis is a re-shoot (`%s`). The sim frames are the same moments as the first shoot when the sim is the same, but the booth's line, any bark bubble and the smoke differ on every run: read them from the new frame before quoting.\n" % prefix)
    outro = os.path.join(here, 'picks-outro.md')
    if os.path.exists(outro): f.write('\n' + open(outro, encoding='utf-8').read().replace('{TAG}', tag.upper()).replace('{tag}', tag))
print('delivered', len(rows), 'to', dest)
