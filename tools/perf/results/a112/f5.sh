#!/bin/sh
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 5 > /dev/null; }
run F5-pro-large-red1065 regular pro large 2000 "opp.hunt_radius_m=20;opp.check_min_closing=1;opp.poke_frames=2;mate_speed_scale=1;red_skill=1.065"
