#!/bin/sh
# lane A95 job 4, round 1: one lever at a time, 600 matches per cell, hockey.regular, paired seeds (A53's)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run L1-pro-base regular pro regular 600
run L1-pro-hktight regular pro regular 600 "opp.hk_share=1;opp.hk_late_frames=0"
run L1-pro-hkpress regular pro regular 600 "opp.hk_read_human_pressure=1"
run L1-pro-check2 regular pro regular 600 "opp.check_min_closing=2"
run L1-pro-hunt8 regular pro regular 600 "opp.hunt_radius_m=8"
run L1-pro-sprint07 regular pro regular 600 "opp.sprint_share=0.7"
run L1-pro-red105 regular pro regular 600 "red_skill=1.05"
run L1-as-base good all_star regular 600
run L1-as-check15 good all_star regular 600 "opp.check_min_closing=1.5"
run L1-as-hunt14 good all_star regular 600 "opp.hunt_radius_m=14"
run L1-as-sprint1 good all_star regular 600 "opp.sprint_share=1"
run L1-as-lcd20 good all_star regular 600 "opp.lift_cooldown_frames=20"
run L1-as-red11 good all_star regular 600 "red_skill=1.1"
run L1-rk-base novice rookie regular 600
run L1-rk-hk085 novice rookie regular 600 "opp.hk_share=0.85;opp.hk_late_frames=8"
run L1-rk-check6 novice rookie regular 600 "opp.check_min_closing=6"
run L1-rk-sprint03 novice rookie regular 600 "opp.sprint_share=0.3"
run L1-rk-red09 novice rookie regular 600 "red_skill=0.9"
run L1-rk-poke14 novice rookie regular 600 "opp.poke_frames=14"
