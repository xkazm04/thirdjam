#!/bin/sh
# lane A112 final, rookie and pro: 2,000 a cell, both rinks, the written profile given as its tune (own build carries the old numbers)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 6 > /dev/null; }
RK="opp.shoot_extra_m=25;opp.poke_frames=6;mate_speed_scale=1"
PRO="opp.hunt_radius_m=20;opp.check_min_closing=1;opp.poke_frames=2;opp.sprint_share=0.7;opp.lift_cooldown_frames=45;mate_speed_scale=1"
run F1-rk-reg novice rookie regular 2000 "$RK"
run F1-pro-reg regular pro regular 2000 "$PRO"
run F1-rk-large novice rookie large 2000 "$RK"
run F1-pro-large regular pro large 2000 "$PRO"
