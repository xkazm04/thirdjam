#!/bin/sh
# lane A133: the six cells as the tree stands (A112's profile, whistle off), 600 a cell
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
run B-rk-reg novice rookie regular 600 &
run B-pro-reg regular pro regular 600 &
run B-as-reg good all_star regular 600 &
wait
run B-rk-large novice rookie large 600 &
run B-pro-large regular pro large 600 &
run B-as-large good all_star large 600 &
wait
