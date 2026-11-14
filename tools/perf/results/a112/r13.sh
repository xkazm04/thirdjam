#!/bin/sh
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
AS="mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60"
run L13-as-carry good all_star regular 600 "$AS;opp.hk_pressure_s=0.1;opp.hk_better_by_m=20"
run L13-as-behind good all_star regular 600 "$AS;opp.check_from_behind=1"
