#!/bin/sh
# lane A95 job 4: the last step on hockey.regular at 2,000 (each cell was just outside its interval at 2,000: 82.7 / 46.0 / 27.9)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run F2-rk-red097 novice rookie regular 2000 "red_skill=0.97"
run F2-pro-red107 regular pro regular 2000 "red_skill=1.07"
run F2-as-sp15 good all_star regular 2000 "speed_scale=1.5"
