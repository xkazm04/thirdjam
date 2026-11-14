#!/bin/sh
# lane A194 chain 2: the three regular cells (600, closing), breakaway speed_bonus 0.2 (main) and 0.3
cd "$(dirname "$0")"
echo "p2 $$" >> out/pids.txt
for b in 0.2 0.3; do
  for c in "rk-reg novice rookie regular" "pro-reg regular pro regular" "as-reg good all_star regular"; do
    set -- $c; node curve.mjs BA$b-$1 $2 $3 $4 600 --workers 1 --check closing --set "skater.breakaway.speed_bonus=$b" > /dev/null
  done
done
echo p2-done
