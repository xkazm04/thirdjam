#!/bin/sh
# lane A95 job 4, round 9: all_star from D2 (hunt 20, sprint 1, better-by 8, pressure 0.8, checks from 2, red 0.9): red 0.8, the carry-or-pass knobs further; 600 per cell
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
D="opp.hunt_radius_m=20;opp.sprint_share=1;opp.check_min_closing=2"
run L9-as-E1 good all_star regular 600 "$D;opp.hk_better_by_m=8;opp.hk_pressure_s=0.8;red_skill=0.8"
run L9-as-E2 good all_star regular 600 "$D;opp.hk_better_by_m=12;opp.hk_pressure_s=0.5;red_skill=0.9"
run L9-as-E3 good all_star regular 600 "$D;opp.hk_better_by_m=12;opp.hk_pressure_s=0.5;red_skill=0.8"
