#!/bin/sh
# lane A162: the six cells on main a7d89871 as built, the ladder's instrument (JAM_PLAYTESTER_CHECK=closing; poke read as the config ships: off), 600 a cell, one cell at a time, three workers
cd "$(dirname "$0")"
for c in "B-rk-reg novice rookie regular" "B-pro-reg regular pro regular" "B-as-reg good all_star regular" "B-rk-large novice rookie large" "B-pro-large regular pro large" "B-as-large good all_star large"; do
  set -- $c; node curve.mjs $1 $2 $3 $4 600 --workers 3 --check closing > /dev/null
done
