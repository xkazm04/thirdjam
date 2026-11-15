#!/bin/sh
# lane A217 CURVE READY 1: determinism repeat, then the six cells at N seeds by the person's path (buttons), then by the verbs (the ladder's instrument).
cd "$(dirname "$0")"
N=${N:-300}; WK=${WK:-2}
echo "c1 $$ $(date +%H:%M)" >> out/pids.txt
node hand.mjs R-pro-7b pro regular 7 --seeds 1-7 --simseed 1 --workers 1 --maxframes 7200 --hashes > /dev/null
node same.mjs R-pro-7 R-pro-7b > out/repeat.txt
for c in "rk rookie novice" "pro pro regular" "as all_star good"; do set -- $c
  for rink in regular large; do r=reg; [ $rink = large ] && r=large
    node hand.mjs B-$1-$r $2 $rink $N --workers $WK > /dev/null; done; done
for c in "rk rookie novice" "pro pro regular" "as all_star good"; do set -- $c
  for rink in regular large; do r=reg; [ $rink = large ] && r=large
    node curve.mjs V-$1-$r $3 $2 $rink $N "" --workers $WK --check closing > /dev/null; done; done
echo "c1 done $(date +%H:%M)" >> out/pids.txt
