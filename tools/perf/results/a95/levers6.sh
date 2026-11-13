#!/bin/sh
# lane A95 job 4, round 6: all_star combinations (the hunt and the sprint with the carry-or-pass knobs, a wider hunt, red skill down); 600 per cell
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run L6-as-C1 good all_star regular 600 "opp.hunt_radius_m=14;opp.sprint_share=1;opp.hk_better_by_m=8;opp.hk_pressure_s=0.8"
run L6-as-C2 good all_star regular 600 "opp.hunt_radius_m=20;opp.sprint_share=1;opp.check_min_closing=2"
run L6-as-C3 good all_star regular 600 "opp.hunt_radius_m=14;opp.sprint_share=1;red_skill=0.9"
