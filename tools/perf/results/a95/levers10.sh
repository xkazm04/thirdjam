#!/bin/sh
# lane A95 job 4, round 10: all_star from D2: the shot's wind-up (18 -> 10), no forecheck yield, the cross-check cooldown 20; 600 per cell
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
D2="opp.hunt_radius_m=20;opp.sprint_share=1;opp.hk_better_by_m=8;opp.hk_pressure_s=0.8;opp.check_min_closing=2;red_skill=0.9"
run L10-as-F1 good all_star regular 600 "$D2;opp.shot_charge_frames=10"
run L10-as-F2 good all_star regular 600 "$D2;opp.forecheck_yield_frames=-1;opp.lift_cooldown_frames=20"
run L10-as-F3 good all_star regular 600 "$D2;opp.shot_charge_frames=10;opp.forecheck_yield_frames=-1;opp.lift_cooldown_frames=20"
