# Lane A211 (the eye): where the human's passes die, from a run's game.log (headless or filmed), for the pass lane.
#   python passdie.py <run dir> [<run dir> ...]
# For each pass of the human with a man on the aim line (JAMEVENT pass who=0 ... kind=to_N; the HUD's pass_meter line of the frame before gives the
# distance to the lit mate): who touches the puck next and when, set against the time the puck needs for that distance at the tap's speed
# (-speed, metres a second, 15 by default: what the stills show):
#   arrived      the next toucher is a mate
#   at release   an opponent has it within 10 frames
#   an opponent, 11 to 30 frames   on the way (at 15 m/s that is 3 to 7 m out)
#   an opponent, after 30 frames   at the receiver's end, or the puck died short and was collected
#   nobody       no toucher in 240 frames (wide, or short and dead)
#   back         the passer himself touches it next
import sys, os, re
speed = 15.0
runs = []
for a in sys.argv[1:]:
    if a.startswith('-speed='): speed = float(a[7:])
    else: runs.append(a)
tot = {}
rows = []
for run in runs:
    log = os.path.join(run, 'game.log')
    if not os.path.exists(log): continue
    ev = []; reach = {}
    for l in open(log, encoding='utf-8', errors='replace'):
        m = re.search(r'JAMHUD frame=(\d+) pass_meter=mate target=(-?\d+) reach_m=([\d.]+)', l)
        if m: reach[int(m.group(1))] = float(m.group(3)); continue
        m = re.search(r'JAMEVENT pass who=0 .*frame=(\d+) kind=(\S+)', l)
        if m: ev.append((int(m.group(1)), 'hp', m.group(2))); continue
        m = re.search(r'JAMPASS frame=(\d+) skater=(\d+) team=(\d+)', l)
        if m: ev.append((int(m.group(1)), 'pass', int(m.group(2)), int(m.group(3)))); continue
        m = re.search(r'JAMVIS takegrace frame=(\d+) skater=(\d+) team=(\d+) human=(\d) on=1', l)
        if m: ev.append((int(m.group(1)), 'take', int(m.group(2)), int(m.group(3)))); continue
        m = re.search(r'JAMRINK react goal end=(\S+) frame=(\d+)', l)
        if m: ev.append((int(m.group(2)), 'goal'))
    ev.sort(key=lambda e: (e[0], 0 if e[1] == 'take' else 1))
    for i, e in enumerate(ev):
        if e[1] != 'hp': continue
        if e[2].endswith('_-1'): tot['no man on the line'] = tot.get('no man on the line', 0) + 1; continue
        d = None
        for f in range(e[0], e[0] - 30, -1):
            if f in reach: d = reach[f]; break
        tap_reach = d   # the HUD's reach_m is how far this press carries (a tap: 11.7 m), not how far the mate is: no distance to the mate in the log
        flight = None
        res, gap = 'nobody', -1
        for n in ev[i + 1:]:
            if n[0] - e[0] > 240 or n[1] == 'goal': break
            who = None
            if n[1] == 'take' and n[0] > e[0]: who = (n[2], n[3])
            elif n[1] == 'pass' and n[2] != 0: who = (n[2], n[3])
            if not who: continue
            gap = n[0] - e[0]
            if who[0] == 0: res = 'back'
            elif who[1] == 0: res = 'arrived'
            elif gap <= 10: res = 'at release'
            elif flight and gap < 0.7 * flight: res = 'on the way'
            elif flight: res = 'at receiver'
            else: res = 'an opponent, 11 to 30 frames' if gap <= 30 else 'an opponent, after 30 frames'   # no HUD line (a headless run): no distance
            break
        tot[res] = tot.get(res, 0) + 1
        rows.append((os.path.basename(os.path.normpath(run)), e[0], e[2], d, round(flight) if flight else None, res, gap))
n = sum(v for k, v in tot.items() if k != 'no man on the line')
print('his passes with a man on the line: %d' % n, {k: v for k, v in sorted(tot.items())})
if '-v' in sys.argv or True:
    for r in rows: print('  %s f%d %s dist %s m, flight %s frames: %s after %d' % r)
