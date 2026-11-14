#!/bin/sh
# lane A189 pass 2, build 8 (wind-ups 8 / 12 + brain8 source; brain8 by tune): law 3 with brain8=1 at pro and all_star (one process) beside the six cells at 600 with brain8=1 (two workers)
cd "$(dirname "$0")"
echo "g1 $$" > ../../../../Game/logs/a189/chain.pid
env -u JAM_PLAYTESTER_CHECK sh law3.sh law3-3 "pro:brain8=1|all_star:brain8=1|pro:-" &
echo "g1-law3 $!" >> ../../../../Game/logs/a189/chain.pid
for c in "V-pro-reg regular pro regular" "V-as-reg good all_star regular" "V-rk-reg novice rookie regular" "V-pro-large regular pro large" "V-as-large good all_star large" "V-rk-large novice rookie large"; do
  set -- $c; node curve.mjs $1 $2 $3 $4 600 "brain8=1" --workers 2 --check closing > /dev/null
done
wait
sed 's/ | lifted.*//' ../../../../Game/logs/a189/law3-3.txt
echo g1-done
