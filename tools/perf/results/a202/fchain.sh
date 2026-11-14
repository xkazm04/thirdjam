#!/bin/sh
# lane A202: on build F (item 4 built off): G1-* the true lead on top of the shipped game (600 a cell), then H8-* the default (100 a cell) against F8-* (build E's shipped game)
cd "$(dirname "$0")"
echo "chain-f $$" >> out/pids.txt
W=2 ./cells.sh G1 600 "opp.hk_true_lead=1;mate.hk_true_lead=1"
W=2 ./cells.sh H8 100 ""
for c in pro-reg as-reg rk-reg pro-large as-large rk-large; do node same.mjs F8-$c H8-$c; done
echo fchain-done
