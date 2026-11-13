#!/bin/sh
# lane A95 job 4, round 5: all_star's shot range and draw, then the combination; 600 per cell
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run L5-as-shoot25 good all_star regular 600 "opp.shoot_extra_m=25"
run L5-as-draw0 good all_star regular 600 "opp.draw_jump_pct=0"
run L5-as-combo good all_star regular 600 "opp.hunt_radius_m=14;opp.sprint_share=1;opp.shoot_extra_m=25;opp.draw_jump_pct=0"
