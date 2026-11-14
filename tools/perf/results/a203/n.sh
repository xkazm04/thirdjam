#!/bin/sh
# lane A203: on the final build (the key ships true): the hash proof of the neutral (--set skater.charge_dies_on_loss=false), 100 seeds a cell, labels N-*,
# against build 0 (main's sim, B-*). Child pids in out/pids.txt.
cd "$(dirname "$0")"
echo "chain-n $$" >> out/pids.txt
for c in "pro-reg regular pro regular" "as-reg good all_star regular" "rk-reg novice rookie regular" "as-large good all_star large" "pro-large regular pro large" "rk-large novice rookie large"; do
  set -- $c; node curve.mjs N-$1 $2 $3 $4 100 --workers ${W:-2} --check closing --set "skater.charge_dies_on_loss=false" > /dev/null
  node same.mjs B-$1 N-$1
done
echo n-done
