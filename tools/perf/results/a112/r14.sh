#!/bin/sh
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
RK="opp.shoot_extra_m=25;opp.poke_frames=6;mate_speed_scale=1"
run L14-rk-large-red102 novice rookie large 600 "$RK;red_skill=1.02"
run L14-rk-large-sp10 novice rookie large 600 "$RK;speed_scale=1.0"
