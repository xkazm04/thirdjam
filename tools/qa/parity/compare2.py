#!/usr/bin/env python3
"""The parity table of protocol 3 (runall2.mjs). usage: compare2.py <out dir> [seeds...] -> markdown on stdout, JSON on stderr.
Per seed: every Development launch (clean = zero stick in every frame, hash of 600..3600, final score from the log), the repeat control among the clean launches,
every Shipping launch (HUD OCR: final score, clock seconds on which the score read agrees with the reference Development launch, first-seen goal clocks)."""
import sys, os, re, json, glob
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
def stick_stats(log):
    """(frames logged up to frame 3600, of those frames with a non-zero stick): what can reach the hashes of frames 600..3600"""
    t = open(log, encoding='utf-8', errors='replace').read()
    fr = [(int(m[1]), m[2]) for m in re.finditer(r'JAMCTRL frame=(\d+) stick=\(([^)]*)\)', t)]
    fr = [x for x in fr if x[0] <= 3600]
    return len(fr), sum(1 for _, v in fr if v != '0.000,0.000')
res = {}
rows_md = []
for s in seeds:
    devs = sorted(glob.glob(os.path.join(out, f'dev-s{s}-r*')))
    ships = sorted(glob.glob(os.path.join(out, f'ship-s{s}-r*')))
    cell = {'seed': s, 'dev': [], 'ship': []}
    ref = None
    for d in devs:
        lp = os.path.join(d, 'game.log')
        if not os.path.exists(lp): continue
        L = dlog(lp); n, bad = stick_stats(lp)
        hs = [h['hash'] for h in L['hashes'] if h['frame'] <= 3600]
        fin = L['hashes'][-1]['score'] if L['hashes'] else '?'
        e = {'run': os.path.basename(d), 'frames_logged': n, 'nonzero_stick_frames': bad, 'clean': n > 1000 and bad == 0, 'hashes': hs, 'final': fin, 'goal_frames': [f for f in L['goal_frames']], 'events': L['events']}
        cell['dev'].append(e)
        if ref is None or e['nonzero_stick_frames'] < ref['nonzero_stick_frames']: ref = {**e, 'dir': d}
    clean = [e for e in cell['dev'] if e['clean']]
    cell['dev_repeat'] = 'n/a (fewer than two clean launches)'
    if len(clean) >= 2:
        a, b = clean[0]['hashes'], clean[1]['hashes']
        cell['dev_repeat'] = f"{sum(1 for x, y in zip(a, b) if x == y)}/{min(len(a), len(b))} hashes equal ({clean[0]['run']} vs {clean[1]['run']})"
    refshots = shots(ref['dir']) if ref else None
    refmap = score_by_clock(refshots['rows']) if refshots else {}
    for d in ships:
        S = shots(d); sb = score_by_clock(S['rows'])
        common = sorted(set(sb) & set(refmap)); agree = [c for c in common if sb[c] == refmap[c]]
        cell['ship'].append({'run': os.path.basename(d), 'final': S['final'], 'common': len(common), 'agree': len(agree), 'goals_first_seen_clock': [v['clock'] for k, v in S['first_seen'].items() if k != '00:00'], 'over_card_t': S['over_card_t']})
    res[s] = cell
    dtxt = '<br>'.join(f"{e['run']}: {'clean' if e['clean'] else 'stick!=0 in %d of %d frames' % (e['nonzero_stick_frames'], e['frames_logged'])}; {' '.join(h[:6] for h in e['hashes'])}; final {e['final']}" for e in cell['dev'])
    stxt = '<br>'.join(f"{e['run']}: final {e['final']}, {e['agree']}/{e['common']} clock-seconds same score, goals first seen at {', '.join(e['goals_first_seen_clock']) or 'none'}" for e in cell['ship'])
    gtxt = ', '.join(f"{f} (0:{int(60 - f/60):02d})" for f in (ref['goal_frames'] if ref else []) if f <= 3600) or 'none'
    rows_md.append(f"| {s} | {dtxt} | {cell['dev_repeat']} | {gtxt} | {stxt} |")
print('| seed | Development launches (hash of frames 600, 1200 .. 3600, first 6 hex) | Development repeat | goals in the reference Development log (frame = match clock) | Shipping launches (HUD stills) |')
print('|---|---|---|---|---|')
print('\n'.join(rows_md))
print(json.dumps(res), file=sys.stderr)
