# Lane A135 (track U1, evolution 4): two runs of the same match with the scripted human steering through the camera (jam.ScriptHumanCam N), one with a
# camera rule on and one with it off. Reads both game logs (the JAMCAM line per sim frame: the human's place px, py, the yaw the hold added kyaw; the JAMSIM
# hash lines) and prints: where the two matches first differ (hash, the human's place), how far apart his two tracks are, how many frames the hold turned
# the camera, and for each run how much he turns at the boards (reversals and degrees of heading per metre within 6 m of a board).
#   python tools/qa/screen/u1-handcmp.py <a/game.log> <b/game.log> <rink_length_m> <rink_width_m> [last_frame=5999]
import re, sys, math
a_path, b_path, L, W = sys.argv[1], sys.argv[2], float(sys.argv[3]), float(sys.argv[4])
last = int(sys.argv[5]) if len(sys.argv) > 5 else 5999
rx = re.compile(r'JAMCAM frame=(\d+) x=(-?[\d.]+) y=(-?[\d.]+) .*? yaw=(-?[\d.]+) .*?px=(-?[\d.]+) py=(-?[\d.]+) kyaw=(-?[\d.]+)')
rh = re.compile(r'JAMSIM frame=(\d+) hash=([0-9a-f]+)')
rhand = re.compile(r'JAMSCREEN hand (.*)')
def read(path):
    cam, hashes, hand = {}, {}, ''
    for line in open(path, encoding='utf-8', errors='replace'):
        m = rx.search(line)
        if m:
            f = int(m.group(1))
            if f <= last and f not in cam:
                cam[f] = (float(m.group(5)), float(m.group(6)), float(m.group(7)), float(m.group(4)), float(m.group(2)), float(m.group(3)))
            continue
        m = rh.search(line)
        if m and int(m.group(1)) <= last + 1: hashes[int(m.group(1))] = m.group(2)
        m = rhand.search(line)
        if m: hand = m.group(1)
    return cam, hashes, hand
def boards(cam):
    # Reversals: the heading (from his own track) turned more than 120 degrees inside 12 frames while he moved at more than 1 m/s; and the heading's
    # total change per metre skated; both only within 6 m of a board.
    fs = sorted(cam)
    head = {}
    for f in fs:
        if f - 3 in cam:
            dx, dy = cam[f][0] - cam[f - 3][0], cam[f][1] - cam[f - 3][1]
            d = math.hypot(dx, dy)
            if d > 0.05 and d < 3.0: head[f] = (math.degrees(math.atan2(dy, dx)), d / 3.0 * 60.0)
    near = lambda f: (L / 2 - abs(cam[f][0]) < 6.0) or (W / 2 - abs(cam[f][1]) < 6.0)
    rev, turn, dist, n, lastrev = 0, 0.0, 0.0, 0, -100
    for f in fs:
        if f not in head or not near(f): continue
        n += 1
        if f - 1 in head:
            turn += abs((head[f][0] - head[f - 1][0] + 180) % 360 - 180)
            dist += head[f][1] / 60.0
        if f - 12 in head and head[f][1] > 1.0 and head[f - 12][1] > 1.0 and abs((head[f][0] - head[f - 12][0] + 180) % 360 - 180) > 120 and f - lastrev > 30:
            rev += 1; lastrev = f
    return n, rev, (turn / dist if dist > 0 else 0.0)
A, HA, handA = read(a_path)
B, HB, handB = read(b_path)
fs = sorted(set(A) & set(B))
print(f"A {a_path}: {len(A)} frames; hand: {handA}")
print(f"B {b_path}: {len(B)} frames; hand: {handB}")
common = sorted(set(HA) & set(HB))
firsth = next((f for f in common if HA[f] != HB[f]), None)
print(f"hashes: {len(common)} common JAMSIM lines (frames {common[0] if common else '-'}..{common[-1] if common else '-'}); first that differs: {firsth if firsth is not None else 'none: the two matches are the same match'}")
d = {f: math.hypot(A[f][0] - B[f][0], A[f][1] - B[f][1]) for f in fs}
firstp = next((f for f in fs if d[f] > 0.02), None)
print(f"his track: {len(fs)} common frames; first frame the two places differ by more than 2 cm: {firstp if firstp is not None else 'none'}; largest distance {max(d.values()) if d else 0:.2f} m"
      + (f"; 60 / 120 / 300 frames after the first difference: " + " / ".join(f"{d.get(firstp + k, float('nan')):.2f} m" for k in (60, 120, 300)) if firstp is not None else ""))
for name, C in (('A', A), ('B', B)):
    ky = [f for f in sorted(C) if abs(C[f][2]) > 0.01]
    lost = [f for f in C if abs(C[f][4]) > 1 / 3 or abs(C[f][5]) > 1 / 3]
    n, rev, tpm = boards(C)
    print(f"{name}: the hold turned the camera on {len(ky)} of {len(C)} frames (at most {max((abs(C[f][2]) for f in ky), default=0):.1f} deg; first {ky[0] if ky else '-'}); chest outside the middle third {len(lost)}; "
          f"within 6 m of a board: {n} frames, {rev} reversals, {tpm:.1f} deg of heading per metre")
