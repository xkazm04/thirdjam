#!/usr/bin/env python3
"""Reduces the stills of one run to what the HUD shows: for every still the blue and orange score, the period, the clock and whether the REMATCH card
is up; then the summary: score at the last still of each period, the first still (seconds after ENTER) of each score, the first REMATCH still, final score.
usage: shots.py <run dir> -> one JSON object"""
import sys, os, json, re, glob
import cv2, pytesseract
from ocr import read, pytesseract as _pt
def rematch(path):
    img = cv2.imread(path)
    c = img[230:350, 460:820]
    g = cv2.cvtColor(c, cv2.COLOR_BGR2GRAY)
    g = cv2.resize(g, None, fx=3, fy=3, interpolation=cv2.INTER_CUBIC)
    _, t = cv2.threshold(g, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    s = pytesseract.image_to_string(t, config='--psm 6') + pytesseract.image_to_string(255 - t, config='--psm 6')
    return 'REMATCH' in s.upper() or 'RESTART' in s.upper()
def summarise(d):
    cache = os.path.join(d, 'shots.json')
    n_png = len(glob.glob(os.path.join(d, 't*.png')))
    if os.path.exists(cache):
        c = json.load(open(cache))
        if c.get('n') == n_png: return c
    r = _summarise(d)
    json.dump(r, open(cache, 'w'))
    return r
def _summarise(d):
    rows = []
    for p in sorted(glob.glob(os.path.join(d, 't*.png'))):
        r = read(p)
        t = int(re.search(r't(\d+)', r['shot'])[1]) / 10.0
        per = int(r['period']) if re.fullmatch(r'[123]', r['period']) else None
        ok = lambda s: bool(re.fullmatch(r'\d\d', s))
        rows.append({'t': t, 'blue': r['blue'] if ok(r['blue']) else None, 'orange': r['orange'] if ok(r['orange']) else None, 'period': per, 'clock': r['clock'] if re.fullmatch(r'\d:\d\d', r['clock']) else None})
    # the rematch card only matters near the end: look at the last 12 stills
    over_t = None
    for p in sorted(glob.glob(os.path.join(d, 't*.png')))[-12:]:
        if rematch(p): over_t = int(re.search(r't(\d+)', os.path.basename(p))[1]) / 10.0; break
    last = [r for r in rows if r['blue'] and r['orange']][-3:]
    final = collections.Counter(f"{r['blue']}:{r['orange']}" for r in last).most_common(1)[0][0] if last else None
    byper = {}
    for r in rows:
        if r['blue'] and r['orange'] and r['period']: byper[r['period']] = f"{r['blue']}:{r['orange']}"
    first = {}
    for r in rows:
        if r['blue'] and r['orange']: first.setdefault(f"{r['blue']}:{r['orange']}", {'t': r['t'], 'period': r['period'], 'clock': r['clock']})
    return {'n': len(rows), 'final': final, 'score_end_of_period': byper, 'first_seen': first, 'over_card_t': over_t, 'rows': rows}
import collections
if __name__ == '__main__':
    s = summarise(sys.argv[1]); print(json.dumps(s))
