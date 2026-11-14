#!/bin/sh
# lane A203: on build 1 (the key in, absent = false): OFF-* (600, the ladder's instrument, key absent: the neutral value; its first 100 seeds are the hash
# proof against B-*), then ON-* (600, --set skater.charge_dies_on_loss=true, the same seeds). Child pids in out/pids.txt.
cd "$(dirname "$0")"
echo "chain-on $$" >> out/pids.txt
for c in "pro-reg regular pro regular" "as-reg good all_star regular" "rk-reg novice rookie regular" "as-large good all_star large" "pro-large regular pro large" "rk-large novice rookie large"; do
  set -- $c; node curve.mjs OFF-$1 $2 $3 $4 600 --workers ${W:-2} --check closing > /dev/null
  node same.mjs B-$1 OFF-$1
  node curve.mjs ON-$1 $2 $3 $4 600 --workers ${W:-2} --check closing --set "skater.charge_dies_on_loss=true" > /dev/null
  node pair.mjs OFF-$1 ON-$1
done
echo on-done
