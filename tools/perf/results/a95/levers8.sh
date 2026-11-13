#!/bin/sh
# lane A95 job 4, round 8: all_star: the carry-or-pass knobs with a 20 m hunt and checks from 2 m/s, then red skill / speed down; 600 per cell
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
D1="opp.hunt_radius_m=20;opp.sprint_share=1;opp.hk_better_by_m=8;opp.hk_pressure_s=0.8;opp.check_min_closing=2"
run L8-as-D1 good all_star regular 600 "$D1"
run L8-as-D2 good all_star regular 600 "$D1;red_skill=0.9"
run L8-as-D3 good all_star regular 600 "$D1;speed_scale=1.05"
