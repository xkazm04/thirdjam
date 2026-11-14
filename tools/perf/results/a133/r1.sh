#!/bin/sh
# lane A133 round 1 (600, regular): pro with the hunt back to 8 m (law 3) and the shot / sprint / lift levers; rookie's offence trimmed down
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 2 > /dev/null; }
run R1-pro-h8 regular pro regular 600 "opp.hunt_radius_m=8" &
run R1-pro-h8-s30 regular pro regular 600 "opp.hunt_radius_m=8;opp.shoot_extra_m=30" &
run R1-pro-h8-s40 regular pro regular 600 "opp.hunt_radius_m=8;opp.shoot_extra_m=40" &
run R1-pro-h8-s40-sp-l regular pro regular 600 "opp.hunt_radius_m=8;opp.shoot_extra_m=40;opp.sprint_share=0.7;opp.lift_cooldown_frames=45" &
wait
run R1-rk-s18 novice rookie regular 600 "opp.shoot_extra_m=18" &
run R1-rk-s15 novice rookie regular 600 "opp.shoot_extra_m=15" &
run R1-rk-p10 novice rookie regular 600 "opp.poke_frames=10" &
run R1-rk-p10-s20 novice rookie regular 600 "opp.poke_frames=10;opp.shoot_extra_m=20" &
wait
