# Lane A158 (track U1, evolution 5): the camera's pitch in two runs of the same match, frame by frame (jam.camera.Log 1: one JAMCAM line a sim frame).
#   python tools/qa/screen/u5-pitch.py <run A game.log> <run B game.log> [last frame, default 5999]
# Only the sim frames both logs carry are compared (a loaded machine skips sim frames between render ticks). Prints, for A and for B: the frames looking
# down steeper than 55.5 / 60 / 70 degrees, the steepest, the highest camera, the frames with the chest outside the middle third (third=0) and the
# largest |x|, |y| of the chest in the frame; then the frames whose yaw differs, and the largest step a frame of pitch and height outside a faceoff reset.
import re, sys

PAT = re.compile(r'JAMCAM frame=(\d+) x=(-?[\d.]+) y=(-?[\d.]+) third=(\d) yaw=(-?[\d.]+) pitch=(-?[\d.]+) lift=(-?[\d.]+) clamp=(-?[\d.]+) h=(-?[\d.]+) px=(-?[\d.]+) py=(-?[\d.]+)')

def read(path, last):
    rows = {}
    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        for line in f:
            m = PAT.search(line)
            if not m:
                continue
            fr = int(m.group(1))
            if fr > last or fr in rows:
                continue
            rows[fr] = dict(x=float(m.group(2)), y=float(m.group(3)), third=int(m.group(4)), yaw=float(m.group(5)), down=-float(m.group(6)), h=float(m.group(9)),
                            px=float(m.group(10)), py=float(m.group(11)))
    return rows

def stats(rows, frames):
    out = dict(n=len(frames), o55=0, o60=0, o70=0, steep=0.0, steep_at=-1, high=0.0, lost=0, mx=0.0, my=0.0, dp=0.0, dh=0.0)
    prev = None
    for fr in frames:
        r = rows[fr]
        if r['down'] > 55.5: out['o55'] += 1
        if r['down'] > 60: out['o60'] += 1
        if r['down'] > 70: out['o70'] += 1
        if r['down'] > out['steep']: out['steep'], out['steep_at'] = r['down'], fr
        out['high'] = max(out['high'], r['h'])
        if r['third'] == 0: out['lost'] += 1
        out['mx'] = max(out['mx'], abs(r['x'])); out['my'] = max(out['my'], abs(r['y']))
        if prev is not None and fr - prev[0] == 1:
            p = rows[prev[0]]
            if abs(r['px'] - p['px']) + abs(r['py'] - p['py']) < 3.0:   # not a faceoff reset
                out['dp'] = max(out['dp'], abs(r['down'] - p['down'])); out['dh'] = max(out['dh'], abs(r['h'] - p['h']))
        prev = (fr,)
    return out

def main():
    a_path, b_path = sys.argv[1], sys.argv[2]
    last = int(sys.argv[3]) if len(sys.argv) > 3 else 5999
    a, b = read(a_path, last), read(b_path, last)
    common = sorted(set(a) & set(b))
    if not common:
        raise SystemExit('no common frames')
    same_path = sum(1 for fr in common if abs(a[fr]['px'] - b[fr]['px']) < 0.005 and abs(a[fr]['py'] - b[fr]['py']) < 0.005)
    yaw = sum(1 for fr in common if abs((a[fr]['yaw'] - b[fr]['yaw'] + 180) % 360 - 180) > 0.05)
    sa, sb = stats(a, common), stats(b, common)
    print('frames in A %d, in B %d, common %d (to sim frame %d); the human at the same place on %d of them; yaw differs on %d' % (len(a), len(b), len(common), last, same_path, yaw))
    for name, s in (('A', sa), ('B', sb)):
        print('%s: steeper than 55.5 deg %d, than 60 deg %d, than 70 deg %d; steepest %.1f deg (frame %d); highest %.2f m; chest outside the middle third %d (max |x| %.3f, |y| %.3f); largest step a frame: pitch %.1f deg, height %.2f m'
              % (name, s['o55'], s['o60'], s['o70'], s['steep'], s['steep_at'], s['high'], s['lost'], s['mx'], s['my'], s['dp'], s['dh']))
    # where the cap bit hardest: the frames with the largest fall in pitch
    falls = sorted(common, key=lambda fr: a[fr]['down'] - b[fr]['down'], reverse=True)[:5]
    print('largest falls in pitch (frame: A -> B deg, height A -> B m): ' + '; '.join('%d: %.1f -> %.1f, %.2f -> %.2f' % (fr, a[fr]['down'], b[fr]['down'], a[fr]['h'], b[fr]['h']) for fr in falls))

main()
