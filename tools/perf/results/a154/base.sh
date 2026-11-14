#!/bin/sh
# lane A154: the six cells on main 4cab387 as built, 600 a cell, one cell at a time, three workers (three processes, -Multiprocess in curve.mjs)
cd "$(dirname "$0")"
for c in "B-rk-reg novice rookie regular" "B-pro-reg regular pro regular" "B-as-reg good all_star regular" "B-rk-large novice rookie large" "B-pro-large regular pro large" "B-as-large good all_star large"; do
  set -- $c; node curve.mjs $1 $2 $3 $4 600 --workers 3 > /dev/null
done
