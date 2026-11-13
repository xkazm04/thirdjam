#!/usr/bin/env python3
"""Writes the five parity cuts: hockey.regular (pro) with seed s and rules {mode: timer, timer_frames: 3600}; nothing else changes.
usage: make-cuts.py <out dir> [seeds...]   (default seeds 1 2 3 4 5)"""
import json, os, sys
here = os.path.dirname(os.path.abspath(__file__))
src = os.path.join(here, '..', '..', '..', 'Game', 'Content', 'Cartridges', 'hockey.regular.cartridge.json')
out = sys.argv[1]
seeds = [int(s) for s in sys.argv[2:]] or [1, 2, 3, 4, 5]
os.makedirs(out, exist_ok=True)
for s in seeds:
    c = json.load(open(src, encoding='utf-8'))
    c['seed'] = s
    c['id'] = 'parity-s%d' % s
    c['title'] = 'Parity seed %d' % s
    r = c['game']['rules']
    r.pop('score_to_win', None)
    r['mode'] = 'timer'
    r['timer_frames'] = 3600
    p = os.path.join(out, 'par.s%d.cartridge.json' % s)
    json.dump(c, open(p, 'w', encoding='utf-8'), indent=2)
    print(p)
