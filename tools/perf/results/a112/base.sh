#!/bin/sh
# lane A112 step 1: the three target cells on both rinks on main 25df19f as built (600 a cell)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
run B-rk-reg novice rookie regular 600
run B-pro-reg regular pro regular 600
run B-as-reg good all_star regular 600
run B-rk-large novice rookie large 600
run B-pro-large regular pro large 600
run B-as-large good all_star large 600
