#!/bin/sh
# lane A95 job 4, round 11: all_star G1 = E2's carry-or-pass knobs + F3 (charge 10, no yield, lift cooldown 20); then speed 1.4; then the mates' share to the human 0.1
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
G1="opp.hunt_radius_m=20;opp.sprint_share=1;opp.hk_better_by_m=12;opp.hk_pressure_s=0.5;opp.check_min_closing=2;red_skill=0.9;opp.shot_charge_frames=10;opp.forecheck_yield_frames=-1;opp.lift_cooldown_frames=20"
run L11-as-G1 good all_star regular 600 "$G1"
run L11-as-G2 good all_star regular 600 "$G1;speed_scale=1.4"
run L11-as-G3 good all_star regular 600 "$G1;opp.shoot_extra_m=50"
