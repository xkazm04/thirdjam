#!/bin/sh
# lane A193: the six cells (600, the ladder's instrument) on build 0 (this worktree's main 7d944ee1, no edit); labels B-*. Child pids in out/pids.txt.
cd "$(dirname "$0")"
echo "chain $$" >> out/pids.txt
for c in "B-pro-reg regular pro regular" "B-as-reg good all_star regular" "B-rk-reg novice rookie regular" "B-as-large good all_star large" "B-pro-large regular pro large" "B-rk-large novice rookie large"; do
  set -- $c; node curve.mjs $1 $2 $3 $4 600 --workers 3 --check closing > /dev/null
done
echo before-done
