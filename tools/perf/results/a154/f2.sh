#!/bin/sh
# lane A154: all_star regular at 2,000: as built and speed 1.45 (tune), three workers each
cd "$(dirname "$0")"
node curve.mjs F-as-reg good all_star regular 2000 --workers 3 > /dev/null &
node curve.mjs F-as-reg-sp145 good all_star regular 2000 "speed_scale=1.45" --workers 3 > /dev/null &
wait
