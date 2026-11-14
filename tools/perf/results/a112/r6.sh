#!/bin/sh
# lane A112 round 6: all_star with the mates at the human's pace (R-0048 point 4), own build (tune key mate_speed_scale)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
run L6-as-mate10 good all_star regular 600 "mate_speed_scale=1"
run L6-as-mate10-shoot60 good all_star regular 600 "mate_speed_scale=1;opp.shoot_extra_m=60;opp.hunt_radius_m=30"
run L6-as-mate10-large good all_star large 600 "mate_speed_scale=1"
