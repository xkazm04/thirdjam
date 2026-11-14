#!/bin/sh
# lane A193: on build 3 (as shipped: true speed on, true gap off): the six cells (S-*, 600, the ladder's instrument). Child pids in out/pids.txt.
cd "$(dirname "$0")"
echo "chain-ship $$" >> out/pids.txt
for c in "pro-reg regular pro regular" "as-reg good all_star regular" "rk-reg novice rookie regular" "as-large good all_star large" "pro-large regular pro large" "rk-large novice rookie large"; do
  set -- $c; node curve.mjs S-$1 $2 $3 $4 600 --workers ${W:-2} --check closing > /dev/null
done
echo ship-done
