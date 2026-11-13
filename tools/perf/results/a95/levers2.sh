#!/bin/sh
# lane A95 job 4, round 2 (pro): the carry-or-pass execution knobs, then red skill and speed; 600 per cell
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run L2-pro-press08 regular pro regular 600 "opp.hk_pressure_s=0.8"
run L2-pro-better8 regular pro regular 600 "opp.hk_better_by_m=8"
run L2-pro-mincarry40 regular pro regular 600 "opp.hk_min_carry_frames=40"
run L2-pro-red115 regular pro regular 600 "red_skill=1.15"
run L2-pro-speed11 regular pro regular 600 "speed_scale=1.1"
run L2-pro-combo regular pro regular 600 "red_skill=1.1;opp.hunt_radius_m=8;opp.check_min_closing=2"
