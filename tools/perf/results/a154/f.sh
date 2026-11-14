#!/bin/sh
# lane A154: 2,000 a cell, one at a time, three workers: the cells within 4 of their bar, the two large trims as tunes on build 1
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 > /dev/null; }
run F-rk-reg novice rookie regular 2000
run F-pro-reg regular pro regular 2000
run F-rk-large novice rookie large 2000
run F-pro-large-red115 regular pro large 2000 "red_skill=1.15"
run F-as-large-sp147 good all_star large 2000 "speed_scale=1.47"
