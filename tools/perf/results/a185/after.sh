#!/bin/sh
# lane A185: on the built tree: A-* the six cells as shipped (free_speed_mult 1.15), then N-* the same at the neutral value 1 (hash identity against B-*). 600, closing.
cd "$(dirname "$0")"
echo "chain-after $$" >> out/pids.txt
for c in "rk-reg novice rookie regular" "pro-reg regular pro regular" "as-reg good all_star regular" "rk-large novice rookie large" "pro-large regular pro large" "as-large good all_star large"; do
  set -- $c; node curve.mjs A-$1 $2 $3 $4 600 --workers 2 --check closing > /dev/null
done
echo after-done
for c in "rk-reg novice rookie regular" "pro-reg regular pro regular" "as-reg good all_star regular" "rk-large novice rookie large" "pro-large regular pro large" "as-large good all_star large"; do
  set -- $c; node curve.mjs N-$1 $2 $3 $4 600 --workers 2 --check closing --set "skater.free_speed_mult=1" > /dev/null
done
echo neutral-done
