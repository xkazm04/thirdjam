#!/bin/sh
# lane A112 round 2: stronger and stacked aggression, hockey.regular, 600 a cell, own build
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
run L2-rk-poke6 novice rookie regular 600 "opp.poke_frames=6"
run L2-rk-check2 novice rookie regular 600 "opp.check_min_closing=2"
run L2-rk-combo novice rookie regular 600 "opp.poke_frames=6;opp.check_min_closing=2;opp.hunt_radius_m=8;opp.sprint_share=0.35"
run L2-pro-combo regular pro regular 600 "opp.hunt_radius_m=20;opp.check_min_closing=1;opp.poke_frames=2;opp.sprint_share=0.7;opp.lift_cooldown_frames=45"
run L2-as-combo good all_star regular 600 "opp.hunt_radius_m=30;opp.check_min_closing=1;opp.lift_cooldown_frames=20"
run L2-rk-red11 novice rookie regular 600 "red_skill=1.1"
