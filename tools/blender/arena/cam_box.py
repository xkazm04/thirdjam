"""cam_box.py LOG: the game camera's REAL poses (JAMCAM lines, jam.camera.Log 1) during every penalty of a live match, and where points of the rink fall
in its picture. Frame fractions as UJamFollowCamera::FrameFraction: x, y in -1..1 inside the picture (FOV 70 horizontal, 16:9), y up."""
import math, re, sys, json
log = sys.argv[1]
cam = {}
fouls = []
rc = re.compile(r"JAMCAM frame=(\d+) x=([-0-9.]+) y=([-0-9.]+) third=\d yaw=([-0-9.]+) pitch=([-0-9.]+) lift=([-0-9.]+) clamp=([-0-9.]+) h=([-0-9.]+) px=([-0-9.]+) py=([-0-9.]+) .* cx=([-0-9.]+) cy=([-0-9.]+)")
rf = re.compile(r"JAMRINK react box team=(\d) who=(-?\d+) frame=(\d+) penalty=(\d+)")
for line in open(log, "r", errors="replace"):
    m = rc.search(line)
    if m:
        f = int(m.group(1))
        cam[f] = dict(fx=float(m.group(2)), fy=float(m.group(3)), yaw=float(m.group(4)), pitch=float(m.group(5)), h=float(m.group(8)), px=float(m.group(9)), py=float(m.group(10)), cx=float(m.group(11)), cy=float(m.group(12)))
        continue
    m = rf.search(line)
    if m:
        fouls.append((int(m.group(3)), int(m.group(1)), int(m.group(2)), int(m.group(4))))
TH = math.tan(math.radians(35.0)); TV = TH * 9.0 / 16.0
def frac(c, p):
    yaw, pitch = math.radians(c["yaw"]), math.radians(c["pitch"])
    f = (math.cos(pitch) * math.cos(yaw), math.cos(pitch) * math.sin(yaw), math.sin(pitch))
    r = (-math.sin(yaw), math.cos(yaw), 0.0)
    u = (-math.sin(pitch) * math.cos(yaw), -math.sin(pitch) * math.sin(yaw), math.cos(pitch))
    d = (p[0] - c["cx"], p[1] - c["cy"], p[2] - c["h"])
    x = sum(a * b for a, b in zip(d, f))
    if x <= 0.2:
        return None
    return (sum(a * b for a, b in zip(d, r)) / x / TH, sum(a * b for a, b in zip(d, u)) / x / TV, x)
HW = 22.5
POINTS = {
    "now: team 0's box seat (-y boards, x -3)": (-2.65, -23.37, 1.1),
    "now: team 1's box seat (-y boards, x +3)": (2.65, -23.37, 1.1),
    "+y boards, x -3": (-2.65, 23.37, 1.1),
    "+y boards, x +3": (2.65, 23.37, 1.1),
    "+x end boards, y -8": (45.87, -8.0, 1.1),
    "+x end boards, y +8": (45.87, 8.0, 1.1),
    "-x end boards, y -8": (-45.87, -8.0, 1.1),
    "-x end boards, y +8": (-45.87, 8.0, 1.1),
}
print("camera frames logged: %d (sim frames %d..%d); fouls: %s" % (len(cam), min(cam), max(cam), fouls))
allf = sorted(cam)
def share(frames, p, margin=0.9):
    n = k = 0
    for f in frames:
        c = cam.get(f)
        if not c: continue
        n += 1
        q = frac(c, p)
        if q and abs(q[0]) <= margin and abs(q[1]) <= margin: k += 1
    return n, k
for (ff, team, who, pen) in fouls:
    frames = [f for f in range(ff + 1, ff + pen)]
    cs = [cam[f] for f in frames if f in cam]
    if not cs: continue
    print("penalty from frame %d (team %d, skater %d, %d frames): %d camera frames; human at (%.1f, %.1f)..(%.1f, %.1f); camera yaw %.0f..%.0f pitch %.0f..%.0f height %.1f..%.1f, at (%.1f, %.1f)..(%.1f, %.1f)" % (
        ff, team, who, pen, len(cs), cs[0]["px"], cs[0]["py"], cs[-1]["px"], cs[-1]["py"], min(c["yaw"] for c in cs), max(c["yaw"] for c in cs), min(c["pitch"] for c in cs), max(c["pitch"] for c in cs),
        min(c["h"] for c in cs), max(c["h"] for c in cs), cs[0]["cx"], cs[0]["cy"], cs[-1]["cx"], cs[-1]["cy"]))
    for name, p in POINTS.items():
        n, k = share(frames, p)
        q = frac(cs[len(cs) // 2], p)
        print("    %-44s in the picture on %4d of %4d frames (%.0f %%); mid-penalty at %s" % (name, k, n, 100.0 * k / max(1, n), ("(%.2f, %.2f) %.0f m off" % q) if q else "behind the camera"))
    mid = cs[len(cs) // 2]
    print("    mid-penalty pose: " + json.dumps(mid))
print("whole match (%d camera frames):" % len(allf))
for name, p in POINTS.items():
    n, k = share(allf, p)
    print("    %-44s in the picture on %5d of %5d frames (%.1f %%)" % (name, k, n, 100.0 * k / max(1, n)))
