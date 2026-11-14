#!/bin/sh
# lane A112 round 8: all_star: mates at the human's pace + shot range + hunt 30, both rinks
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
AS="mate_speed_scale=1;opp.hunt_radius_m=30"
run L8-as-shoot80 good all_star regular 600 "$AS;opp.shoot_extra_m=80"
run L8-as-shoot60-large good all_star large 600 "$AS;opp.shoot_extra_m=60"
run L8-as-shoot60-chg6 good all_star regular 600 "$AS;opp.shoot_extra_m=60;opp.shot_charge_frames=6"
