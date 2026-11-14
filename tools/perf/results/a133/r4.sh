#!/bin/sh
# lane A133 round 4 (600, large): rookie red skill, pro red skill with the carry
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 2 > /dev/null; }
C="opp.hunt_radius_m=8;opp.hk_pressure_s=0.3;opp.hk_better_by_m=12"
run R4-rk-large-p10-red112 novice rookie large 600 "opp.poke_frames=10;red_skill=1.12" &
run R4-rk-large-p10-red116 novice rookie large 600 "opp.poke_frames=10;red_skill=1.16" &
run R4-pro-large-carry-red113 regular pro large 600 "$C;red_skill=1.13" &
run R4-pro-large-carry-red116 regular pro large 600 "$C;red_skill=1.16" &
wait
