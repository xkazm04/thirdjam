#!/bin/sh
# lane A95 job 4: the large rink's three target cells at 2,000 with the interpolated offsets (seeds 1-2,000 of hockey.arena.large)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run F3-rk-large novice rookie large 2000 "red_skill=0.86"
run F3-pro-large regular pro large 2000 "red_skill=1.06"
run F3-as-large good all_star large 2000 "speed_scale=1.25"
