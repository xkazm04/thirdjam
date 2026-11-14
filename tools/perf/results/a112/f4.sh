#!/bin/sh
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 5 > /dev/null; }
run F4-as-reg-draw25 good all_star regular 2000 "mate_speed_scale=1;opp.hunt_radius_m=30;opp.shoot_extra_m=60;opp.draw_jump_pct=25"
run F4-rk-large-red098 novice rookie large 2000 "opp.shoot_extra_m=25;opp.poke_frames=6;mate_speed_scale=1;red_skill=0.98"
