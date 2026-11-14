#!/bin/sh
# lane A162: R-0060's second column (item 4): the six cells with the course press (JAM_PLAYTESTER_CHECK=course, as the config ships), 600 a cell, same seeds as base.sh
cd "$(dirname "$0")"
for c in "P-rk-reg novice rookie regular" "P-pro-reg regular pro regular" "P-as-reg good all_star regular" "P-rk-large novice rookie large" "P-pro-large regular pro large" "P-as-large good all_star large"; do
  set -- $c; node curve.mjs $1 $2 $3 $4 600 --workers 3 --check course > /dev/null
done
