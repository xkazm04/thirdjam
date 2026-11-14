#!/bin/sh
# lane A202: item 3, do the bots brace? opp.cross_hold_closing at two values, pro and all_star, both rinks, 600 seeds (labels X4-*, X8-*)
cd "$(dirname "$0")"
echo "chain-brace $$" >> out/pids.txt
W=2 ./cells.sh X4 600 "opp.cross_hold_closing=4" "pro as"
W=2 ./cells.sh X8 600 "opp.cross_hold_closing=8" "pro as"
echo brace-done
