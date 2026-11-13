#!/usr/bin/env python3
"""Prints the two tables of PARITY.md from results/parity3.compare.json (compare2.py's JSON): the five-seed parity table and the Development event counts."""
import json, sys
d = json.load(open(sys.argv[1] if len(sys.argv) > 1 else 'results/parity3.compare.json'))
print('| seed | Dev hash 600 / 1200 / 1800 / 2400 / 3000 / 3600 (first 8 hex) | Dev launches with the same 6 hashes | Dev final | Ship final per launch | clock seconds read in both with the same score | goals: Dev sim frame (log) | goals: Ship first still showing the new score (match clock) |')
print('|---|---|---|---|---|---|---|---|')
for s, c in d.items():
    dev = c['dev']; ref = min(dev, key=lambda e: e['nonzero_stick_frames'])
    same = sum(1 for e in dev if e['hashes'] == ref['hashes'])
    def clk(f): return None
    ships = c['ship']
    fin = ', '.join(f"{e['final'].replace('0', '', 0)}" for e in ships)
    agree = ', '.join(f"{e['agree']}/{e['common']}" for e in ships)
    goals = ', '.join(str(f) for f in ref['goal_frames'] if f <= 3600) or 'none'
    sg = '; '.join(', '.join(e['goals_first_seen_clock']) or 'none' for e in ships)
    print(f"| {s} | {' '.join(h[:8] for h in ref['hashes'])} | {same} of {len(dev)} | {ref['final']} | {fin} | {agree} | {goals} | {sg} |")
print()
print('| seed | reference Dev launch | goals | shots | saves | posts | checks (lift) | pokes | breakaways | faceoffs | board hits |')
print('|---|---|---|---|---|---|---|---|---|---|---|')
for s, c in d.items():
    ref = min(c['dev'], key=lambda e: e['nonzero_stick_frames']); ev = ref['events']
    print(f"| {s} | {ref['run']} | {ev.get('goal', 0)} | {ev.get('shot', 0)} | {ev.get('save', 0)} | {ev.get('post', 0)} | {ev.get('lift', 0)} | {ev.get('poke', 0)} | {ev.get('breakaway', 0)} | {ev.get('faceoff', 0)} | {ev.get('board', 0)} |")
