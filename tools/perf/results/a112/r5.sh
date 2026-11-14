#!/bin/sh
# lane A112 round 5: the candidates on the large rink, and the rookie's shot range trimmed
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
PRO="opp.hunt_radius_m=20;opp.check_min_closing=1;opp.poke_frames=2;opp.sprint_share=0.7;opp.lift_cooldown_frames=45"
run L5-rk-large novice rookie large 600 "opp.shoot_extra_m=30;opp.poke_frames=6"
run L5-pro-large regular pro large 600 "$PRO"
run L5-rk-shoot25 novice rookie regular 600 "opp.shoot_extra_m=25;opp.poke_frames=6"
