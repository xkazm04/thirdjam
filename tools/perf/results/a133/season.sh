#!/bin/sh
# lane A133: the season in order on the ladder profile (build 4), the novice and regular players, takes 3-8 at their own levels, 200 a take;
# plus takes 4 and 5 as candidate intermediate steps (rookie / pro with one ai number) for the novice
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 2 > /dev/null; }
for t in 3 4 5 6 7 8; do run S-novice-take-$t novice cart ladder/take-$t 200 & done; wait
run S4-novice-t4-rookie novice rookie ladder/take-4 200 &
run S4-novice-t5-rookie novice rookie ladder/take-5 200 &
run S4-novice-t4-pro-red09 novice cart ladder/take-4 200 --set "ai.red_skill=0.9" &
run S4-novice-t5-pro-red095 novice cart ladder/take-5 200 --set "ai.red_skill=0.95" &
wait
