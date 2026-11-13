#!/bin/sh
# lane A95 job 4: all_star on hockey.regular at 2,000 after speed 1.5 read 30.7 (worse than 1.4's 27.9): speed 1.4 with the bots' lift cooldown 30 (the sim's own), or pressure 0.3 s
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run F4-as-lcd30 good all_star regular 2000 "speed_scale=1.4;opp.lift_cooldown_frames=30"
run F4-as-press03 good all_star regular 2000 "speed_scale=1.4;opp.hk_pressure_s=0.3"
