#!/bin/sh
# lane A112 round 9: all_star speed last, per rink (mates at 1.0, hunt 30, shot range 60)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
AS="mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60"
run L9-as-large-sp127 good all_star large 600 "$AS;speed_scale=1.27"
run L9-as-sp13 good all_star regular 600 "$AS;speed_scale=1.3"
run L9-as-large-sp132 good all_star large 600 "$AS;speed_scale=1.32"
