#!/bin/sh
# lane A133: the six cells as built (build 3, the profile, no tune), 2,000 a cell
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 > /dev/null; }
run F-rk-reg novice rookie regular 2000 &
run F-pro-reg regular pro regular 2000 &
run F-as-reg good all_star regular 2000 &
run F-rk-large novice rookie large 2000 &
run F-pro-large regular pro large 2000 &
run F-as-large good all_star large 2000 &
wait
