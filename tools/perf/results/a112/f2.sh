#!/bin/sh
# lane A112 final all_star regular (2,000) and one more speed arm (600)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 5 > /dev/null; }
AS="mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60"
run F2-as-reg good all_star regular 2000 "$AS"
run L10-as-sp145 good all_star regular 600 "$AS;speed_scale=1.45"
