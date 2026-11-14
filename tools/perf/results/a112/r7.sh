#!/bin/sh
# lane A112 round 7: all_star, speed last (mates at the human's pace), hockey.regular
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
run L7-as-m1-red10 good all_star regular 600 "mate_speed_scale=1;red_skill=1.0"
run L7-as-m1-sp15 good all_star regular 600 "mate_speed_scale=1;speed_scale=1.5"
run L7-as-m1-red08 good all_star regular 600 "mate_speed_scale=1;red_skill=0.8"
