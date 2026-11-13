# Lane A135 (track U1): the projection probe's numbers from a game log. Reads the JAMCAM lines (jam.camera.Log 1: one per sim frame, the chest's place in
# the frame, x and y in -1..1; the middle third is |x|, |y| <= 0.333) of sim frames [first, last] and prints how many frames had the chest outside the
# middle third, outside the frame, the largest |x| and |y|, and the longest run of frames outside.
#   python tools/qa/screen/u1-camstat.py <game.log> [last_frame=5999] [first_frame=0]
import re, sys
path = sys.argv[1]
last = int(sys.argv[2]) if len(sys.argv) > 2 else 5999
first = int(sys.argv[3]) if len(sys.argv) > 3 else 0
rx = re.compile(r'JAMCAM frame=(\d+) x=(-?[\d.]+) y=(-?[\d.]+) third=(\d) yaw=(-?[\d.]+) pitch=(-?[\d.]+)')
seen = {}
for line in open(path, encoding='utf-8', errors='replace'):
    m = rx.search(line)
    if not m: continue
    f = int(m.group(1))
    if first <= f <= last and f not in seen:
        seen[f] = (float(m.group(2)), float(m.group(3)), float(m.group(5)), float(m.group(6)))
frames = sorted(seen)
out3 = [f for f in frames if abs(seen[f][0]) > 1/3 or abs(seen[f][1]) > 1/3]
outf = [f for f in frames if abs(seen[f][0]) > 1 or abs(seen[f][1]) > 1]
runs, run, prev = [], 0, None
for f in out3:
    run = run + 1 if prev is not None and f == prev + 1 else 1
    if run == 1: runs.append([f, 1])
    else: runs[-1][1] = run
    prev = f
mx = max((abs(seen[f][0]) for f in frames), default=0)
my = max((abs(seen[f][1]) for f in frames), default=0)
dp = [abs(seen[b][3] - seen[a][3]) for a, b in zip(frames, frames[1:]) if b == a + 1]
dy = [abs((seen[b][2] - seen[a][2] + 180) % 360 - 180) for a, b in zip(frames, frames[1:]) if b == a + 1]
dp.sort(); dy.sort()
p = lambda v, q: v[min(len(v) - 1, int(len(v) * q))] if v else 0
print(f"{path}: frames {len(frames)} ({frames[0] if frames else '-'}..{frames[-1] if frames else '-'}); chest outside the middle third {len(out3)}; outside the frame {len(outf)}; "
      f"max |x| {mx:.3f}, max |y| {my:.3f}; pitch step per frame p99 {p(dp, 0.99):.2f} max {p(dp, 1.0):.2f} deg; yaw step p99 {p(dy, 0.99):.2f} max {p(dy, 1.0):.2f} deg")
runs.sort(key=lambda r: -r[1])
print("  longest runs outside (first frame, frames): " + ", ".join(f"{a}+{n}" for a, n in runs[:10]))
