#!/bin/sh
# lane A189 pass-2 prep: all_star large with the dial (600, two workers) beside law 3 with the dial (one process)
cd "$(dirname "$0")"
echo "e2 $$" > ../../../../Game/logs/a189/chain.pid
env -u JAM_PLAYTESTER_CHECK sh law3.sh law3-2 "all_star:opp.hk_second_man_hunts=1|all_star:opp.hk_second_man_hunts=1;opp.hk_step_up_max_team=99;opp.hk_step_up_m=16" &
node curve.mjs Z-as-large-sm good all_star large 600 "opp.hk_second_man_hunts=1" --workers 2 --check closing > /dev/null
node curve.mjs Z-as-large-dial16 good all_star large 600 "opp.hk_second_man_hunts=1;opp.hk_step_up_max_team=99;opp.hk_step_up_m=16" --workers 2 --check closing > /dev/null
wait
sed 's/ | lifted.*//' ../../../../Game/logs/a189/law3-2.txt
echo e2-done
