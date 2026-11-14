#!/bin/sh
# lane A112 final pro, softened (sprint and lift cooldown back to A95's 0.35 / 90): 2,000 both rinks
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 5 > /dev/null; }
PRO2="opp.hunt_radius_m=20;opp.check_min_closing=1;opp.poke_frames=2;mate_speed_scale=1"
run F3-pro-reg regular pro regular 2000 "$PRO2"
run F3-pro-large regular pro large 2000 "$PRO2"
