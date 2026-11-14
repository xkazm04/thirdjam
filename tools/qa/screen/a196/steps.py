import sys, re, collections
# steps.py <hit-lines.txt> : the human's step through each wind-up (JAMHIT step lines): per press the largest step across his facing, the largest
# part of a step along it that his own speed does not explain (the pull), and the largest change of the drawn step from one frame to the next. Lane A210.
rx = re.compile(r'JAMHIT step frame=(\d+) since_press=(\d+) windup=(\d+) along_cm=([-\d.]+) across_cm=([-\d.]+) speed_mps=([-\d.]+) yaw_step_deg=([-\d.]+)')
press = collections.OrderedDict()
for line in open(sys.argv[1], encoding='utf-8', errors='replace'):
    m = rx.search(line)
    if not m: continue
    f, since, wu = int(m.group(1)), int(m.group(2)), int(m.group(3))
    al, ac, sp, yw = (float(m.group(i)) for i in (4, 5, 6, 7))
    press.setdefault(f - since, []).append((since, wu, al, ac, sp, yw))
print('press  frames  max_across_cm  max_pull_along_cm  max_step_jump_cm  max_yaw_deg  speed_at_press')
tot = []
for p, rows in press.items():
    rows = [r for r in rows if r[0] <= r[1]]
    # up to the contact only: the first frame whose speed falls by over a third against the frame before is the hit; from there the step is the hit's
    cut = len(rows)
    for i in range(1, len(rows)):
        if rows[i][4] < rows[i - 1][4] * 0.67: cut = i; break
    rows = rows[:cut]
    step = [(r[2] ** 2 + r[3] ** 2) ** 0.5 for r in rows]
    own = [rows[max(i - 1, 0)][4] * 100.0 / 60.0 for i in range(len(rows))]   # the speed he had coming into the frame
    pull = max(r[2] - o for r, o in zip(rows, own))
    jump = max([abs(step[i] - step[i - 1]) for i in range(1, len(step))] or [0])
    across = max(abs(r[3]) for r in rows)
    yaw = max(abs(r[5]) for r in rows)
    tot.append((across, pull, jump, yaw))
    print(f'{p:6d} {len(rows):6d} {across:13.1f} {pull:18.1f} {jump:17.1f} {yaw:12.1f} {rows[0][4]:14.2f}')
if tot:
    print('worst ', ' '.join(f'{max(t[i] for t in tot):.1f}' for i in range(4)), '(across, pull along, step jump, yaw)')
