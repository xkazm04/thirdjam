#!/bin/sh
# lane A203: on build 0 (this worktree's main 972ba542, sim unedited): the hash baseline, 100 seeds a cell, labels B-*. Child pids in out/pids.txt.
cd "$(dirname "$0")"
echo "chain-b $$" >> out/pids.txt
for c in "pro-reg regular pro regular" "as-reg good all_star regular" "rk-reg novice rookie regular" "as-large good all_star large" "pro-large regular pro large" "rk-large novice rookie large"; do
  set -- $c; node curve.mjs B-$1 $2 $3 $4 100 --workers ${W:-2} --check closing > /dev/null
done
echo b-done
