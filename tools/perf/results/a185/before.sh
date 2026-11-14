#!/bin/sh
# lane A185: the six cells (600, the ladder's instrument) on the morning build of main 943df8e7, before any edit is built; label B-*
cd "$(dirname "$0")"
echo "chain $$" >> out/pids.txt
for c in "B-rk-reg novice rookie regular" "B-pro-reg regular pro regular" "B-as-reg good all_star regular" "B-rk-large novice rookie large" "B-pro-large regular pro large" "B-as-large good all_star large"; do
  set -- $c; node curve.mjs $1 $2 $3 $4 600 --workers 2 --check closing > /dev/null
done
echo before-done
