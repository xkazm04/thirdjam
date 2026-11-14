#!/bin/sh
# lane A112 round 3: the rookie has to score: shot range, poke, draw; hockey.regular, 600 a cell, own build
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
run L3-rk-shoot20 novice rookie regular 600 "opp.shoot_extra_m=20"
run L3-rk-shoot30-poke6 novice rookie regular 600 "opp.shoot_extra_m=30;opp.poke_frames=6"
run L3-rk-shoot30-poke0-lift novice rookie regular 600 "opp.shoot_extra_m=30;opp.poke_frames=0;opp.lift=1;opp.lift_cooldown_frames=90"
run L3-rk-draw50 novice rookie regular 600 "opp.shoot_extra_m=30;opp.poke_frames=6;opp.draw_jump_pct=50"
