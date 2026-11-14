#!/bin/sh
# lane A112 round 12: rookie large rink offset with the mates at 1.0 (red skill: the large rink's only per-rink lever)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
RK="opp.shoot_extra_m=25;opp.poke_frames=6;mate_speed_scale=1"
run L12-rk-large-red093 novice rookie large 600 "$RK;red_skill=0.93"
run L12-rk-large-red096 novice rookie large 600 "$RK;red_skill=0.96"
