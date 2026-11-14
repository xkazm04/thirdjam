#!/bin/sh
# lane A193: on build 1: the hash proof (brain7=0, 100 seeds a cell, labels N-*), then the six cells as built (A-*, 600, the ladder's instrument). Child pids in out/pids.txt.
cd "$(dirname "$0")"
echo "chain-after $$" >> out/pids.txt
for c in "pro-reg regular pro regular" "as-reg good all_star regular" "rk-reg novice rookie regular" "as-large good all_star large" "pro-large regular pro large" "rk-large novice rookie large"; do
  set -- $c; node curve.mjs N-$1 $2 $3 $4 100 "brain7=0" --workers ${W:-2} --check closing > /dev/null
done
echo hash-done
for c in "pro-reg regular pro regular" "as-reg good all_star regular" "rk-reg novice rookie regular" "as-large good all_star large" "pro-large regular pro large" "rk-large novice rookie large"; do
  set -- $c; node curve.mjs A-$1 $2 $3 $4 600 --workers ${W:-2} --check closing > /dev/null
done
echo after-done
