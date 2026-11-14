#!/bin/sh
# lane A133 round 3 (600): the large rink with the regular picks (rookie poke 10; pro hunt 8 + carry through pressure), its red skill / speed; pro regular carry + shot 40
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 2 > /dev/null; }
C="opp.hunt_radius_m=8;opp.hk_pressure_s=0.3;opp.hk_better_by_m=12"
run R3-rk-large-p10-red103 novice rookie large 600 "opp.poke_frames=10;red_skill=1.03" &
run R3-rk-large-p10-red108 novice rookie large 600 "opp.poke_frames=10;red_skill=1.08" &
run R3-pro-large-carry regular pro large 600 "$C" &
run R3-pro-large-carry-red11 regular pro large 600 "$C;red_skill=1.1" &
wait
run R3-as-large-sp138 good all_star large 600 "speed_scale=1.38" &
run R3-as-large-sp142 good all_star large 600 "speed_scale=1.42" &
run R3-pro-reg-carry-s40 regular pro regular 600 "$C;opp.shoot_extra_m=40" &
run R3-rk-reg-p8 novice rookie regular 600 "opp.poke_frames=8" &
wait
