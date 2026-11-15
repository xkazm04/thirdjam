"""tools/images/modes-prepare.py: the picked MODES illustrations at 1280 x 720, and their contact sheet (Third Jam, track U6, lane A231).

  python -I tools/images/modes-prepare.py [--scratch <dir>] [--sheet-only]

Reads tools/images/modes-picks.json ({"<id>": {"file": "<scratch file name>", "try": 1, "name": "GLOW ICE", "look": "l3_neon"}};
an optional "soften": [[x0, y0, x1, y1, radius]] blurs a small patch of the scratch file, see soften()),
takes each picked file from the generator's scratch folder (default: <TEMP>/thirdjam-images/modes, where
`node tools/images/gen.mjs tools/images/modes-jobs.json` writes), scales it to cover 1280 x 720 (a centred crop of what is over, Lanczos),
writes tools/images/modes/<id>.png (RGB, no alpha) and then docs/experiments/modeart/contact.jpg: the pictures in the order of MODES,
each at 640 x 360 with its name, and under it the same picture at 400 x 225 (the size it must still read at).
Needs Pillow. tools/images/import-modes.py imports the PNGs into the game.
"""
import json
import os
import sys
import tempfile

from PIL import Image, ImageDraw, ImageFilter, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
OUT = os.path.join(HERE, "modes")
SHEET = os.path.join(ROOT, "docs", "experiments", "modeart", "contact.jpg")
ORDER = ["arsenal", "bruisers", "cage", "fog_of_war", "glass_house", "marathon", "outnumbered", "pond", "power_play", "shotgun", "sniper",
         "sudden_death", "rinks", "make_your_own"]
W, H = 1280, 720


def font(size):
    for name in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def soften(img, boxes):
    """A pick's "soften": [[x0, y0, x1, y1, radius], ...] in the scratch file's own pixels: a feathered blur over a small patch
    (clay strokes that could be taken for a mark at a close look). Nothing is painted in."""
    img = img.convert("RGB")
    for x0, y0, x1, y1, radius in boxes:
        patch = img.crop((x0, y0, x1, y1)).filter(ImageFilter.GaussianBlur(radius))
        mask = Image.new("L", patch.size, 0)
        inset = int(radius * 2)
        ImageDraw.Draw(mask).rectangle((inset, inset, patch.width - 1 - inset, patch.height - 1 - inset), fill=255)
        img.paste(patch, (x0, y0), mask.filter(ImageFilter.GaussianBlur(radius)))
    return img


def cover(img):
    img = img.convert("RGB")
    scale = max(W / img.width, H / img.height)
    size = (max(W, round(img.width * scale)), max(H, round(img.height * scale)))
    img = img.resize(size, Image.LANCZOS)
    left, top = (img.width - W) // 2, (img.height - H) // 2
    return img.crop((left, top, left + W, top + H))


def main():
    args = sys.argv[1:]
    scratch = os.path.join(tempfile.gettempdir(), "thirdjam-images", "modes")
    if "--scratch" in args:
        scratch = args[args.index("--scratch") + 1]
    sheet_only = "--sheet-only" in args
    with open(os.path.join(HERE, "modes-picks.json"), "r", encoding="utf-8") as fh:
        picks = json.load(fh)
    os.makedirs(OUT, exist_ok=True)
    os.makedirs(os.path.dirname(SHEET), exist_ok=True)
    made = []
    for ident in ORDER:
        pick = picks.get(ident)
        target = os.path.join(OUT, ident + ".png")
        if pick and not sheet_only:
            src = os.path.join(scratch, pick["file"])
            if not os.path.exists(src):
                print("MISSING %s: %s is not in the scratch folder" % (ident, pick["file"]))
            else:
                with Image.open(src) as img:
                    was = img.size
                    cover(soften(img, pick.get("soften", []))).save(target, "PNG", optimize=True)
                print("OK %s: %s %dx%d -> tools/images/modes/%s.png %dx%d, %.2f MB" % (ident, pick["file"], was[0], was[1], ident, W, H,
                                                                                   os.path.getsize(target) / 1048576.0))
        if os.path.exists(target):
            made.append(ident)

    # The contact sheet: five columns; a tile is the picture at 640 x 360 with its label, and the same at 400 x 225 under it.
    cols, tile_w, pad, label_h, small_w, small_h = 5, 640, 16, 44, 400, 225
    tile_h = label_h + 360 + 8 + small_h
    rows = (len(ORDER) + cols - 1) // cols
    sheet = Image.new("RGB", (pad + cols * (tile_w + pad), pad + rows * (tile_h + pad)), (10, 14, 26))
    draw = ImageDraw.Draw(sheet)
    big, small = font(26), font(16)
    for i, ident in enumerate(ORDER):
        x, y = pad + (i % cols) * (tile_w + pad), pad + (i // cols) * (tile_h + pad)
        pick = picks.get(ident, {})
        label = pick.get("name", ident.upper())
        draw.text((x, y + 6), "%d  %s" % (i + 1, label), fill=(245, 246, 248), font=big)
        note = "%s   try %s" % (pick.get("look", ""), pick.get("try", "?")) if ident in made else "not drawn yet"
        draw.text((x + tile_w - 8, y + 16), note, fill=(163, 171, 184), font=small, anchor="ra")
        if ident in made:
            with Image.open(os.path.join(OUT, ident + ".png")) as img:
                sheet.paste(img.resize((640, 360), Image.LANCZOS), (x, y + label_h))
                sheet.paste(img.resize((small_w, small_h), Image.LANCZOS), (x, y + label_h + 360 + 8))
        else:
            draw.rectangle((x, y + label_h, x + 639, y + label_h + 359), outline=(74, 80, 92))
    sheet.save(SHEET, "JPEG", quality=88)
    print("SHEET docs/experiments/modeart/contact.jpg %dx%d, %d of %d pictures, %.2f MB" % (sheet.width, sheet.height, len(made), len(ORDER),
                                                                                         os.path.getsize(SHEET) / 1048576.0))


main()
