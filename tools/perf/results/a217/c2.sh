#!/bin/sh
# lane A217 (Director 04:08): the six cells by BUTTONS at 300 (after c1's B-rk-reg ends), then the verbs arm at 100 a cell.
cd "$(dirname "$0")"
echo "c2 $$ $(date +%H:%M)" >> out/pids.txt
until grep -q '"label":"B-rk-reg"' cells.jsonl 2>/dev/null; do sleep 10; done
for c in "rk-large rookie large" "pro-reg pro regular" "pro-large pro large" "as-reg all_star regular" "as-large all_star large"; do set -- $c
  node hand.mjs B-$1 $2 $3 300 --workers 2 > /dev/null; done
for c in "rk rookie novice" "pro pro regular" "as all_star good"; do set -- $c
  for rink in regular large; do r=reg; [ $rink = large ] && r=large
    node curve.mjs V-$1-$r $3 $2 $rink 100 "" --workers 2 --check closing > /dev/null; done; done
echo "c2 done $(date +%H:%M)" >> out/pids.txt
