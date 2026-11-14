"""cam_grid.py LOG [LOG2]: for places all round the boards, the share of the game camera's real frames (JAMCAM) in which a boxed figure standing there
(1.1 m up, 0.9 m outside the board line) is inside the picture AND within 30 m (a 1.8 m figure is then 82 px or more of 1080-line picture's 1920 width)."""
import math, re, sys
rc = re.compile(r"JAMCAM frame=(\d+) .* yaw=([-0-9.]+) pitch=([-0-9.]+) .* h=([-0-9.]+) px=([-0-9.]+) py=([-0-9.]+) .* cx=([-0-9.]+) cy=([-0-9.]+)")
rf = re.compile(r"JAMRINK react box team=(\d) who=(-?\d+) frame=(\d+) penalty=(\d+)")
TH = math.tan(math.radians(35.0)); TV = TH * 9.0 / 16.0
def load(log):
    cam, fouls = {}, []
    for line in open(log, "r", errors="replace"):
        m = rc.search(line)
        if m:
            cam[int(m.group(1))] = (float(m.group(2)), float(m.group(3)), float(m.group(4)), float(m.group(7)), float(m.group(8)), float(m.group(5)), float(m.group(6)))
            continue
        m = rf.search(line)
        if m: fouls.append((int(m.group(3)), int(m.group(1)), int(m.group(2)), int(m.group(4))))
    return cam, fouls
def seen(c, p, far=30.0, margin=0.9):
    yaw, pitch = math.radians(c[0]), math.radians(c[1])
    f = (math.cos(pitch) * math.cos(yaw), math.cos(pitch) * math.sin(yaw), math.sin(pitch))
    d = (p[0] - c[3], p[1] - c[4], p[2] - c[2])
    x = d[0] * f[0] + d[1] * f[1] + d[2] * f[2]
    if x <= 0.5 or x > far: return False
    r = (-math.sin(yaw), math.cos(yaw), 0.0)
    u = (-math.sin(pitch) * math.cos(yaw), -math.sin(pitch) * math.sin(yaw), math.cos(pitch))
    return abs((d[0] * r[0] + d[1] * r[1]) / x / TH) <= margin and abs((d[0] * u[0] + d[1] * u[1] + d[2] * u[2]) / x / TV) <= margin
HL, HW, OUT = 45.0, 22.5, 0.9
places = [("-y x %+d" % x, (x, -(HW + OUT), 1.1)) for x in (-36, -27, -18, -9, -3, 3, 9, 18, 27, 36)] + [("+y x %+d" % x, (x, HW + OUT, 1.1)) for x in (-36, -27, -18, 18, 27, 36)] \
       + [("+x end y %+d" % y, (HL + OUT, y, 1.1)) for y in (-15, -10, -6, 6, 10, 15)] + [("-x end y %+d" % y, (-(HL + OUT), y, 1.1)) for y in (-15, -10, -6, 6, 10, 15)]
for log in sys.argv[1:]:
    cam, fouls = load(log)
    frames = sorted(cam)
    boxed_human = set()
    pens = []
    for ff, team, who, pen in fouls:
        rng = [f for f in range(ff + 1, ff + pen) if f in cam]
        # the human is the boxed one when his own position is pinned at the box side for the whole penalty
        pinned = all(abs(cam[f][5]) < 3.2 and cam[f][6] < -21.0 for f in rng[:50])
        (boxed_human.update(rng) if pinned else pens.append(rng))
    free = [f for f in frames if f not in boxed_human]
    pen_frames = [f for rng in pens for f in rng]
    print("%s: %d camera frames, %d with the human free, %d inside a bot's penalty, %d with the human boxed" % (log.split("/")[-1], len(frames), len(free), len(pen_frames), len(boxed_human)))
    rows = []
    for name, p in places:
        a = sum(1 for f in free if seen(cam[f], p)) / max(1, len(free))
        b = sum(1 for f in pen_frames if seen(cam[f], p)) / max(1, len(pen_frames)) if pen_frames else float("nan")
        h = sum(1 for f in boxed_human if seen(cam[f], p)) / max(1, len(boxed_human)) if boxed_human else float("nan")
        rows.append((a, name, b, h))
    for a, name, b, h in sorted(rows, reverse=True):
        print("   %-16s human free %5.1f %%   a bot's penalty %5.1f %%   human boxed %5.1f %%" % (name, 100 * a, 100 * b, 100 * h))
