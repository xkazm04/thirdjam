# Lane A214: motion inside every-frame windows of a run (JAMCAM): largest camera step, pitch step, yaw step, and ice share per frame.
import re, math, sys
from geom import ice_share
pat = re.compile(r'JAMCAM frame=(\d+) .*?px=(\S+) py=(\S+) .*?fyaw=(\S+) fpitch=(\S+) fcx=(\S+) fcy=(\S+) fh=(\S+) pkx=(\S+) pky=(\S+) hyaw=(\S+) bw=(\S+) rl=(\S+) rw=(\S+) rr=(\S+)')
rows = {}
for l in open(sys.argv[1], encoding='utf-8', errors='replace'):
    m = pat.search(l)
    if m: v = [float(x) for x in m.groups()]; rows[int(v[0])] = v
for w in sys.argv[2:]:
    a, b = [int(x) for x in w.split('-')]
    st = ps = ys = 0.0; ices = []; steep = 0
    for f in range(a, b):
        if f not in rows: continue
        r = rows[f]; ices.append(ice_share((r[5], r[6], r[7]), r[3], r[4], r[12], r[13], r[14], cols=24, rows=14)); steep += -r[4] > 55
        if f - 1 in rows:
            q = rows[f - 1]
            if math.hypot(r[1] - q[1], r[2] - q[2]) < 1:
                st = max(st, math.dist(r[5:8], q[5:8])); ps = max(ps, abs(r[4] - q[4])); ys = max(ys, abs((r[3] - q[3] + 180) % 360 - 180))
    print(f'{sys.argv[1]} {a}-{b}: largest step {st*100:.0f} cm, pitch step {ps:.2f} deg, yaw step {ys:.2f} deg, ice min {min(ices):.2f} mean {sum(ices)/len(ices):.2f}, frames > 55 deg {steep}')
