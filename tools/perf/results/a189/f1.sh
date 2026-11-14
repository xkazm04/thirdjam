#!/bin/sh
# lane A189 pass 2 on 38766fc9 + A200's bots' wind-ups 8 / 12 (build 7), 600, closing: the six cells as built; all_star regular's dial (second man + step-up 13 / 16, max team 99); pro regular's hunt 11 / 12 (presser keys)
cd "$(dirname "$0")"
echo "f1 $$" > ../../../../Game/logs/a189/chain.pid
for c in "W-pro-reg regular pro regular" "W-as-reg good all_star regular" "W-rk-reg novice rookie regular" "W-pro-large regular pro large" "W-as-large good all_star large" "W-rk-large novice rookie large"; do
  set -- $c; node curve.mjs $1 $2 $3 $4 600 --workers 3 --check closing > /dev/null
done
for m in 16 13; do node curve.mjs W-as-reg-dial$m good all_star regular 600 "opp.hk_second_man_hunts=1;opp.hk_step_up_max_team=99;opp.hk_step_up_m=$m" --workers 3 --check closing > /dev/null; done
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
for h in 11 12; do node curve.mjs W-pro-reg-h$h regular pro regular 600 "$K;opp.hunt_radius_m=$h" --workers 3 --check closing > /dev/null; done
echo f1-done
