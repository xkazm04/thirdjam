#!/bin/sh
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
AS="mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60"
run L11-as-chg14 good all_star regular 600 "$AS;opp.shot_charge_frames=14"
run L11-as-mpass0 good all_star regular 600 "$AS;mate.pass_to_human=0"
