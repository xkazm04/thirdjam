#!/bin/sh
# lane A170, the Director's 07:30 ask: what the dial moves outside the two cuts. Closing press, as built vs the tune, paired seeds, three workers, one chain.
# 1. the season's takes that play pro (5, 6, 7) at their cartridge's level, novice and regular, 200 a take; 2. the eleven modes at pro, regular class, seeds 1-100.
cd "$(dirname "$0")"
T="opp.hunt_radius_m=11;opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
for k in 5 6 7; do for c in novice regular; do
  node curve.mjs S-$c-take-$k $c cart ladder/take-$k 200 --workers 3 --check closing > /dev/null
  node curve.mjs SD-$c-take-$k $c cart ladder/take-$k 200 "$T" --workers 3 --check closing > /dev/null
done; done
echo takes-done
for m in arsenal bruisers cage fog-of-war glass-house marathon outnumbered pond power-play shotgun sniper; do
  node curve.mjs M-$m regular cart modes/hockey.mode.$m 100 --workers 3 --check closing > /dev/null
  node curve.mjs MD-$m regular cart modes/hockey.mode.$m 100 "$T" --workers 3 --check closing > /dev/null
done
echo t-done
