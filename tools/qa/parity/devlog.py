#!/usr/bin/env python3
"""Summarises a Development game.log of a parity run: cut and seed, the JAMSIM hash line of every 600 frames, the event counts, the goal frames, the last frame.
usage: devlog.py <game.log> -> one JSON object"""
import sys, re, json, collections
def summarise(path):
    sims, ev, goals, frames = [], collections.Counter(), [], 0
    cut = seed = None
    for l in open(path, encoding='utf-8', errors='replace'):
        m = re.search(r'JAMSIM frame=(\d+) hash=([0-9a-f]+) score=(\d+):(\d+) phase=(\d+)', l)
        if m: sims.append({'frame': int(m[1]), 'hash': m[2], 'score': f'{m[3]}:{m[4]}', 'phase': int(m[5])}); continue
        m = re.search(r'JAMCART loaded .*id=(\S+) .*seed=(\d+)', l)
        if m: cut, seed = m[1], int(m[2]); continue
        m = re.search(r'JAMEVENT (\w+) who=(-?\d+) .*frame=(\d+) kind=(\S+)', l)
        if m:
            ev[m[1]] += 1; frames = max(frames, int(m[3]))
            if m[1] == 'goal': goals.append(int(m[3]))
    return {'cut': cut, 'seed': seed, 'hashes': sims, 'events': dict(ev), 'goal_frames': goals, 'last_event_frame': frames}
if __name__ == '__main__': print(json.dumps(summarise(sys.argv[1])))
