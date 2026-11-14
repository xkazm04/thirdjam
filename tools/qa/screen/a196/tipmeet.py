# Lane A196 (evolution 10): from a game log's JAMHIT lines, for every landed press with a chevron target: frames between the tip meeting the man
# (tip_met) and the contact (press_to_contact), by wind-up and by who pressed. Usage: tipmeet.py <game.log> [...]
import re, sys, collections
pat = re.compile(r'JAMHIT frame=(\d+) kind=(\w+) result=(\w+) checker=(-?\d+) victim=(-?\d+) human=([01]) closing=[\d.]+ burst_m=[\d.]+ windup=(\d+) press_to_contact=(-?\d+) tip_met=(-?\d+)')
rows = collections.defaultdict(list)
for f in sys.argv[1:]:
    for line in open(f, encoding='utf-8', errors='replace'):
        m = pat.search(line)
        if not m: continue
        fr, kind, res, chk, vic, hum, wu, ptc, tip = m.groups()
        if kind != 'check' or int(ptc) < 0: continue
        who = 'human-pressed' if line.find('human=1') >= 0 and chk == '0' else 'bot'
        rows[(int(wu), who)].append((int(ptc), int(tip), res))
for (wu, who), r in sorted(rows.items()):
    met = [p - t for p, t, _ in r if t >= 0]
    within = sum(1 for d in met if abs(d) <= 2)
    print(f'windup {wu:2d} {who:14s} landed {len(r):3d}  tip met {len(met):3d}  within 2 frames of contact {within:3d}  '
          f'press->contact {sorted(p for p,_,_ in r)}  contact-tip {sorted(met)}')
