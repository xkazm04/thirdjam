#!/bin/sh
# lane A189 pass 1 (600, closing): rookie regular (base, red 0.85 / 0.80 / 0.75), all_star (speed 1.5 / 1.55 regular; 1.55 / 1.62 large), pro regular hunt 11 / 12 / 13 (presser keys)
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a189/chain.pid
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
node curve.mjs B-rk-reg novice rookie regular 600 --workers 3 --check closing > /dev/null
for r in 0.85 0.80 0.75; do node curve.mjs R-rk-reg-red$r novice rookie regular 600 "red_skill=$r" --workers 3 --check closing > /dev/null; done
for s in 1.5 1.55; do node curve.mjs A-as-reg-sp$s good all_star regular 600 "speed_scale=$s" --workers 3 --check closing > /dev/null; done
for s in 1.55 1.62; do node curve.mjs A-as-large-sp$s good all_star large 600 "speed_scale=$s" --workers 3 --check closing > /dev/null; done
for h in 12 11 13; do node curve.mjs P-pro-reg-h$h regular pro regular 600 "$K;opp.hunt_radius_m=$h" --workers 3 --check closing > /dev/null; done
echo p1-done
