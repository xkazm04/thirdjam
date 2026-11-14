#!/bin/sh
# lane A154: all_star regular speed 1.45 / 1.5 (600, tunes)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 > /dev/null; }
run T2-as-reg-sp145 good all_star regular 600 "speed_scale=1.45" &
run T2-as-reg-sp150 good all_star regular 600 "speed_scale=1.5" &
wait
