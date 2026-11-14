#!/bin/sh
# lane A154 trims, 600 a cell (tunes on build 1), two cells at a time, three workers each
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 > /dev/null; }
run T1-pro-large-red115 regular pro large 600 "red_skill=1.15" &
run T1-as-large-sp145 good all_star large 600 "speed_scale=1.45" &
wait
run T1-pro-large-red117 regular pro large 600 "red_skill=1.17" &
run T1-as-large-sp147 good all_star large 600 "speed_scale=1.47" &
wait
run T1-as-reg-red095 good all_star regular 600 "red_skill=0.95" &
run T1-as-reg-red085 good all_star regular 600 "red_skill=0.85" &
wait
