#!/bin/sh
# lane A95 job 4, round 4: combinations at pro, all_star's shot timing / forecheck yield / pressure read, rookie with red 0.95; 600 per cell
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run L4-as-charge6 good all_star regular 600 "opp.shot_charge_frames=6"
run L4-as-yieldnever good all_star regular 600 "opp.forecheck_yield_frames=-1"
run L4-as-nopress good all_star regular 600 "opp.hk_read_human_pressure=0"
run L4-pro-A regular pro regular 600 "opp.shoot_extra_m=30;red_skill=1.1;opp.hunt_radius_m=8;opp.check_min_closing=2"
run L4-pro-yieldnever regular pro regular 600 "opp.forecheck_yield_frames=-1"
run L4-pro-charge4 regular pro regular 600 "opp.shot_charge_frames=4"
run L4-rk-combo95 novice rookie regular 600 "red_skill=0.95;opp.hk_share=0.85;opp.hk_late_frames=8;opp.check_min_closing=6;opp.sprint_share=0.3"
