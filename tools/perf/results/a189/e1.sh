#!/bin/sh
# lane A189 pass-2 prep on 38766fc9 + the StepUpM restore (build 6): 1. neutral, 100 seeds a cell vs this morning's rows; 2. all_star regular's brain dial by tune (600, closing)
cd "$(dirname "$0")"
echo "e1 $$" > ../../../../Game/logs/a189/chain.pid
for c in "Z-rk-reg novice rookie regular S1-rk-reg" "Z-pro-reg regular pro regular N-pro-reg" "Z-as-reg good all_star regular N-as-reg" "Z-rk-large novice rookie large S1-rk-large" "Z-pro-large regular pro large S1-pro-large" "Z-as-large good all_star large N-as-large"; do
  set -- $c; node curve.mjs $1 $2 $3 $4 100 --workers 3 --check closing > /dev/null; node same.mjs $1 $5
done
node curve.mjs Z-as-reg-sm good all_star regular 600 "opp.hk_second_man_hunts=1" --workers 3 --check closing > /dev/null
for m in 16 10 22; do node curve.mjs Z-as-reg-dial$m good all_star regular 600 "opp.hk_second_man_hunts=1;opp.hk_step_up_max_team=99;opp.hk_step_up_m=$m" --workers 3 --check closing > /dev/null; done
echo e1-done
