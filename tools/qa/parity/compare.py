#!/usr/bin/env python3
"""The parity table. usage: compare.py <out dir with ship-sN, dev-sN, dev2-sN> [seeds...] -> markdown on stdout, JSON of the cells on stderr.
Development side: the JAMSIM hash of every 600 frames and the goal frames from game.log. Shipping side (no log): HUD stills read by OCR (shots.py).
A goal at sim frame F shows as match clock 60 - F/60 seconds (the clock is the 3600-frame timer)."""
import sys, os, json, math
from devlog import summarise as dlog
from shots import summarise as shots
out = sys.argv[1]
seeds = [int(s) for s in sys.argv[2:]] or [1, 2, 3, 4, 5]
def clock_s(c): m, s = c.split(':'); return int(m) * 60 + int(s)
def score_by_clock(rows):
    d = {}
    for r in rows:
        if r['clock'] and r['blue'] and r['orange']: d[clock_s(r['clock'])] = f"{int(r['blue'])}:{int(r['orange'])}"
    return d
res = {}
print('| seed | Dev hashes at 600..3600 (first 8 hex) | Dev repeat | Dev log final (3600) | Dev OCR final | Ship OCR final | goals Dev log (frame = clock) | goals Ship OCR (first still, clock) | common clock seconds, same score | REMATCH card (s after ENTER) dev / ship |')
print('|---|---|---|---|---|---|---|---|---|---|')
for s in seeds:
    cell = {}
    dd = os.path.join(out, f'dev-s{s}'); sd = os.path.join(out, f'ship-s{s}'); d2 = os.path.join(out, f'dev2-s{s}')
    if not (os.path.isdir(dd) and os.path.isdir(sd)): continue
    L = dlog(os.path.join(dd, 'game.log')); hs = {h['frame']: h for h in L['hashes']}
    first = [hs[f]['hash'] for f in (600, 1200, 1800, 2400, 3000, 3600) if f in hs]
    rep = '-'
    if os.path.isdir(d2) and os.path.exists(os.path.join(d2, 'game.log')):
        L2 = dlog(os.path.join(d2, 'game.log')); h2 = {h['frame']: h['hash'] for h in L2['hashes']}
        same = [f for f in hs if f in h2 and h2[f] == hs[f]['hash']]; rep = f"{len(same)}/{len(hs)} hashes equal"
    S = shots(sd); D = shots(dd)
    sb, db = score_by_clock(S['rows']), score_by_clock(D['rows'])
    common = sorted(set(sb) & set(db)); agree = [c for c in common if sb[c] == db[c]]
    goals = [f"{f} (0:{int(60 - f/60):02d})" for f in L['goal_frames'] if f <= 3600]
    sg = [f"{v['clock']}" for k, v in S['first_seen'].items() if k != '00:00']
    log3600 = hs.get(3600, {}).get('score', '?')
    cell = {'seed': s, 'hashes': first, 'repeat': rep, 'dev_log_3600': log3600, 'dev_ocr_final': D['final'], 'ship_ocr_final': S['final'], 'common': len(common), 'agree': len(agree), 'over': [D['over_card_t'], S['over_card_t']]}
    res[s] = cell
    print(f"| {s} | {' '.join(h[:8] for h in first)} | {rep} | {log3600} | {D['final']} | {S['final']} | {', '.join(goals) or 'none'} | {', '.join(sg) or 'none'} | {len(agree)}/{len(common)} | {D['over_card_t']} / {S['over_card_t']} |")
print(json.dumps(res), file=sys.stderr)
