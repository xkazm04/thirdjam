#!/bin/sh
# lane A217 CURVE READY 2: the six cells at 300 by the person's path on the applied tree of 05:08 (build 3).
cd "$(dirname "$0")"
echo "c5 $$ $(date +%H:%M)" >> out/pids.txt
for c in "rk rookie" "pro pro" "as all_star"; do set -- $c
  for rink in regular large; do r=reg; [ $rink = large ] && r=large
    node hand.mjs C-$1-$r $2 $rink 300 --workers 2 > /dev/null; done; done
echo "c5 done $(date +%H:%M)" >> out/pids.txt
