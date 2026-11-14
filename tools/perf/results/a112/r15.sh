#!/bin/sh
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
run L15-as-nolift good all_star regular 600 "mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;opp.lift=0"
run F5-as-large-sp132 good all_star large 2000 "mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;speed_scale=1.32"
