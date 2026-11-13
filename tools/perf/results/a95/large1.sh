#!/bin/sh
# lane A95 job 4: the large rink's offsets (LargeRedSkill / LargeSpeedScale per level), 600 per cell on hockey.arena.large seeds 1-600
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run LG-rk-red08 novice rookie large 600 "red_skill=0.8"
run LG-rk-red08sp08 novice rookie large 600 "red_skill=0.8;speed_scale=0.8"
run LG-pro-red11 regular pro large 600 "red_skill=1.1"
run LG-pro-red12 regular pro large 600 "red_skill=1.2"
run LG-as-sp12 good all_star large 600 "speed_scale=1.2"
run LG-as-sp12red10 good all_star large 600 "speed_scale=1.2;red_skill=1.0"
