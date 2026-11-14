# Lane A186 (track U1, evolution 7): the held cursor on the game's own stills (jam.screen.StillEvery N, with the camera log jam.camera.Log 1).
#   find:  python tools\qa\screen\u7-stills.py find <game.log> [--secs 10]
#          the 10-second stretches where the human is nearest the boards and the camera turns or corrects most (sim frames).
#   mark:  python tools\qa\screen\u7-stills.py mark <game.log> <shots dir> <out dir> --from F --to T [--every 2] [--rink-l 90 --rink-w 45]
#          On every still from F to T: a white cross at the pixel the cursor is held at (over the ice point 2.5 m ahead-right of the human at frame F),
#          an orange ring where that ice point (moved with him) is now. A hand that did not move keeps the cross; the ring is where his aim was.
#          The line from his feet is the blade as the game reads the held cursor (CursorAimFromRay); the label: frame, the blade's turn since F.
#   pair:  python tools\qa\screen\u7-stills.py pair <dirA> <dirB> <out dir> --frames f1,f2,...   (before | after side by side, labelled)
import math, os, re, sys
from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import importlib.util
_spec = importlib.util.spec_from_file_location('u7b', os.path.join(os.path.dirname(os.path.abspath(__file__)), 'u7-bound.py'))
u7b = importlib.util.module_from_spec(_spec); _spec.loader.exec_module(u7b)


def arg(args, name, default, cast=str):
    if name in args:
        i = args.index(name); v = cast(args[i + 1]); del args[i:i + 2]; return v
    return default


def find(path, secs, L=90.0, W=45.0):
    rows = u7b.read(path)
    frames = sorted(rows)
    n = int(secs * 60)
    best = []
    for s in range(0, len(frames) - n, 60):
        win = frames[s:s + n]
        near = 0; yawc = 0.0; corner = 0
        for f in win:
            cam, hm, rig, _ = rows[f]
            dx, dy = L / 2 - abs(hm[0]), W / 2 - abs(hm[1])
            if min(dx, dy) < 4.0: near += 1
            if dx < 8.0 and dy < 8.0: corner += 1
            if rig: yawc += abs(u7b.unwind(cam[3] - rig[3]))
        best.append((near + 2 * corner, win[0], win[-1], near, corner, yawc / n))
    best.sort(reverse=True)
    for b in best[:8]:
        print(f"frames {b[1]}..{b[2]}: near the boards {b[3]} frames, in a corner {b[4]}, mean camera-vs-rig yaw {b[5]:.1f} deg")


def font(sz):
    for f in ('C:/Windows/Fonts/consola.ttf', 'C:/Windows/Fonts/arial.ttf'):
        if os.path.exists(f):
            return ImageFont.truetype(f, sz)
    return ImageFont.load_default()


RINK = [90.0, 45.0]


def mark(path, shots, out, f0, f1, every, label, hold=30):
    """Each still at frame f: the hand held still since f - hold (half a second). The white cross is the pixel the cursor was put on at f - hold (over
    the point 2.5 m ahead of him, inside the boards); the orange ring is that ice point carried with him to frame f (where the cross would sit had the camera only
    followed him). The yellow line is the blade the game reads from the held cross now. The label: the blade's turn over that half second that the
    camera added (the camera's turn of it minus the open-ice rig's, from the log's rig pose)."""
    rows = u7b.read(path)
    os.makedirs(out, exist_ok=True)
    files = {int(m.group(1)): os.path.join(shots, n) for n in os.listdir(shots) for m in [re.search(r'-f0*(\d+)\.png$', n)] if m}
    fnt = font(22)
    done = 0
    adds, log = [], []
    for f in range(f0, f1 + 1):
        F = f - hold
        if f not in rows or F not in rows or (f - f0) % every:
            continue
        src = files.get(f)
        if not src:
            continue
        camF, hF, rigF, _ = rows[F]
        cam, hm, rig, _ = rows[f]
        yr = math.radians(camF[3])
        # 2.5 m ahead of him in the camera's yaw, kept 0.6 m inside the boards (along the glass the ahead-right point of the model lies past it)
        ax = max(-(RINK[0] / 2 - 0.6), min(RINK[0] / 2 - 0.6, hF[0] + math.cos(yr) * 2.5))
        ay = max(-(RINK[1] / 2 - 0.6), min(RINK[1] / 2 - 0.6, hF[1] + math.sin(yr) * 2.5))
        d0 = (ax - hF[0], ay - hF[1])
        c0 = u7b.proj(camF, (hF[0] + d0[0], hF[1] + d0[1], 0))
        if not c0 or math.hypot(*d0) < 0.8:
            continue
        added = None
        a0, a1 = u7b.blade(camF, c0, hF), u7b.blade(cam, c0, hm)
        cr = u7b.proj(rigF, (hF[0] + d0[0], hF[1] + d0[1], 0)) if rigF else None
        if a0 is not None and a1 is not None and cr and rig:
            r0, r1 = u7b.blade(rigF, cr, hF), u7b.blade(rig, cr, hm)
            if r0 is not None and r1 is not None:
                added = u7b.unwind(a1 - a0) - u7b.unwind(r1 - r0)
                adds.append(abs(added)); log.append((f, added))
        im = Image.open(src).convert('RGB')
        sx, sy = im.width / u7b.W, im.height / u7b.H
        dr = ImageDraw.Draw(im)
        P = lambda p: (im.width / 2 + p[0] * sx, im.height / 2 + p[1] * sy)
        ring = u7b.proj(cam, (hm[0] + d0[0], hm[1] + d0[1], 0))
        if ring:
            rx, ry = P(ring); dr.ellipse([rx - 11, ry - 11, rx + 11, ry + 11], outline=(255, 140, 0), width=4)
        cx, cy = P(c0)
        dr.line([cx - 14, cy, cx + 14, cy], fill=(255, 255, 255), width=3); dr.line([cx, cy - 14, cx, cy + 14], fill=(255, 255, 255), width=3)
        feet = u7b.proj(cam, (hm[0], hm[1], 0))
        if feet and a1 is not None:
            tip = u7b.proj(cam, (hm[0] + 1.6 * math.cos(math.radians(a1)), hm[1] + 1.6 * math.sin(math.radians(a1)), 0))
            if tip:
                dr.line([P(feet), P(tip)], fill=(255, 220, 0), width=4)
        txt = f"{label}  sim frame {f}  hand still 0.5 s: the camera turned the blade {added:+.1f} deg" if added is not None else f"{label}  sim frame {f}"
        dr.rectangle([8, im.height - 40, 16 + dr.textlength(txt, font=fnt), im.height - 8], fill=(0, 0, 0))
        dr.text((12, im.height - 38), txt, fill=(255, 255, 255), font=fnt)
        im.save(os.path.join(out, f"{label}-f{f:05d}.png"))
        done += 1
    with open(os.path.join(out, 'added.csv'), 'w') as fh:
        fh.write('sim_frame,camera_added_blade_deg_over_half_second' + chr(10))
        for f, v in log: fh.write(f"{f},{v:.2f}" + chr(10))
    adds.sort()
    q = lambda p: adds[min(len(adds) - 1, int(p * (len(adds) - 1) + 0.5))] if adds else 0.0
    print(f"marked {done} stills into {out}; the blade's turn the camera added over the held half second: median {q(0.5):.1f}, p90 {q(0.9):.1f}, worst {q(1.0):.1f} deg ({len(adds)} stills)")


def pair(da, db, out, frames):
    os.makedirs(out, exist_ok=True)
    for f in frames:
        fa = [n for n in os.listdir(da) if n.endswith(f"-f{f:05d}.png")]
        fb = [n for n in os.listdir(db) if n.endswith(f"-f{f:05d}.png")]
        if not fa or not fb:
            print('missing', f); continue
        A, B = Image.open(os.path.join(da, fa[0])), Image.open(os.path.join(db, fb[0]))
        im = Image.new('RGB', (A.width + B.width, max(A.height, B.height)))
        im.paste(A, (0, 0)); im.paste(B, (A.width, 0))
        im = im.resize((im.width // 2, im.height // 2), Image.LANCZOS)
        im.save(os.path.join(out, f"boards-f{f:05d}-before-after.jpg"), quality=88)
    print('paired', len(frames))


if __name__ == '__main__':
    a = sys.argv[1:]
    mode = a.pop(0)
    if mode == 'find':
        secs = arg(a, '--secs', 10.0, float); L = arg(a, '--rink-l', 90.0, float); Wd = arg(a, '--rink-w', 45.0, float)
        find(a[0], secs, L, Wd)
    elif mode == 'mark':
        RINK[0] = arg(a, '--rink-l', 90.0, float); RINK[1] = arg(a, '--rink-w', 45.0, float)
        f0 = arg(a, '--from', 0, int); f1 = arg(a, '--to', 0, int); ev = arg(a, '--every', 2, int); lab = arg(a, '--label', 'after')
        mark(a[0], a[1], a[2], f0, f1, ev, lab)
    elif mode == 'pair':
        fr = [int(x) for x in arg(a, '--frames', '', str).split(',') if x]
        pair(a[0], a[1], a[2], fr)
