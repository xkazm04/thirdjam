#!/bin/sh
# lane A209 wave 3 (neutral against M1N = d0d132bf with the keys neutral), the candidate $1 (built): the person-like hand's bars, the neutral proof, law 3 beside the six cells at 600, then the gate.
cd "$(dirname "$0")"
P=$1
echo "$(date +%H:%M:%S) chain2-$P sh $$" >> out/pids.txt
for lv in rk pro; do case $lv in rk) c=novice; l=rookie;; pro) c=regular; l=pro;; esac
  node curve.mjs $P-person-$lv $c $l regular 100 "" --workers 2 --check closing --hand person > /dev/null; done
node hand-table.mjs $P-person-rk $P-person-pro > out/$P-person.json
EXTRA="--seam check_reach=0;check_pull=0;cross_any=0;check_windup=4;cross_windup=4" sh hashes.sh $P-N
for c in rk pro as; do for r in reg large; do node same.mjs M1N-$c-$r $P-N-$c-$r; done; done > out/$P-neutral.txt
( EXTRA_ENV="JAM_LAW3_AT=8,8.5,9,9.5" sh law3.sh $P-law3 "pro:-|all_star:-" ) &
W=2 sh cells.sh $P-c600 600
wait
echo "chain2-$P done $(date +%H:%M:%S)"
