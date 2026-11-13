"""Lane A58: draw an Insights-style timeline still of one frame from an Insights timing-events export
(TimingInsights.ExportTimingEvents ... -columns=ThreadName,TimerName,StartTime,EndTime,Duration,Depth).
The Insights UI itself cannot be driven here (a system dialog holds the foreground, so no input reaches its window);
this draws the same data: one lane per thread, nested scopes by depth, labels on scopes wide enough to read.

  python -I tools/perf/timeline.py <hitch-N.csv> <out.png> [--threads GameThread,RenderThread,...] [--maxdepth 9] [--title "..."]
"""
import csv, sys, argparse
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

ap = argparse.ArgumentParser()
ap.add_argument('csv'); ap.add_argument('out')
ap.add_argument('--threads', default='GameThread,RenderThread 1,RHIThread,GPU0-Graphics0')
ap.add_argument('--maxdepth', type=int, default=10)
ap.add_argument('--title', default='')
a = ap.parse_args()

rows = []
with open(a.csv, newline='', encoding='utf-8') as f:
    r = csv.reader(f); h = next(r)
    for c in r:
        if len(c) < 6: continue
        name = ','.join(c[1:len(c) - 4])
        rows.append((c[0], name, float(c[-4]), float(c[-3]), float(c[-2]) * 1000, int(c[-1])))
threads_all = sorted({x[0] for x in rows})
want = [t for t in a.threads.split(',') if t in threads_all]
if not want: want = [t for t in threads_all if 'Game' in t or 'Render' in t][:4]
t0 = min(x[2] for x in rows if x[0] == want[0]); t1 = max(x[3] for x in rows if x[0] == want[0])
span = (t1 - t0) * 1000
palette = ['#4e79a7', '#f28e2b', '#59a14f', '#e15759', '#76b7b2', '#edc948', '#b07aa1', '#ff9da7', '#9c755f', '#bab0ac']
lane_h = a.maxdepth + 1
fig, ax = plt.subplots(figsize=(19.2, 2.2 + 0.32 * lane_h * len(want)), dpi=100)
for li, th in enumerate(want):
    base = (len(want) - 1 - li) * (lane_h + 1)
    ax.text(-span * 0.005, base + lane_h - 0.5, th, ha='right', va='center', fontsize=9, fontweight='bold')
    for (t, n, s, e, d, z) in rows:
        if t != th or z > a.maxdepth: continue
        x = (s - t0) * 1000; w = (e - s) * 1000
        y = base + lane_h - 1 - z
        col = palette[hash(n) % len(palette)]
        ax.add_patch(Rectangle((x, y), max(w, span * 0.0005), 0.9, facecolor=col, edgecolor='white', linewidth=0.3))
        if w > span * 0.035:
            ax.text(x + w / 2, y + 0.45, f'{n[:int(w / span * 160)]} {d:.1f}ms', ha='center', va='center', fontsize=6.5, color='white', clip_on=True)
ax.set_xlim(-span * 0.002, span * 1.002); ax.set_ylim(-0.5, len(want) * (lane_h + 1))
ax.set_yticks([]); ax.set_xlabel('ms from the frame start')
ax.set_title(a.title or a.csv, fontsize=11, loc='left')
for s in ['top', 'right', 'left']: ax.spines[s].set_visible(False)
plt.tight_layout(); plt.savefig(a.out); print('saved', a.out, f'{span:.1f} ms', want)
