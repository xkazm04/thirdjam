#!/bin/sh
# lane A209 wave 1: the neutral proof (old values through the seam), then the person-like hand at rookie and pro, 100 matches, three arms.
cd "$(dirname "$0")"
echo "$(date +%H:%M:%S) chain1 sh $$" >> out/pids.txt
EXTRA="--seam check_reach=0;check_pull=0;cross_any=0;check_windup=4;cross_windup=4" sh hashes.sh N1
for c in rk pro as; do for r in reg large; do node same.mjs B0-$c-$r N1-$c-$r; done; done
for arm in full wu4 nopull; do
  case $arm in full) S="";; wu4) S="check_windup=4;cross_windup=4";; nopull) S="check_reach=0;check_pull=0;cross_any=0";; esac
  for lv in rk pro; do
    case $lv in rk) cls=novice; lvl=rookie;; pro) cls=regular; lvl=pro;; esac
    if [ -n "$S" ]; then node curve.mjs W1-$arm-$lv $cls $lvl regular 100 "" --workers 3 --check closing --hand person --seam "$S" > /dev/null
    else node curve.mjs W1-$arm-$lv $cls $lvl regular 100 "" --workers 3 --check closing --hand person > /dev/null; fi
  done
done
node hand-table.mjs W1-full-rk W1-full-pro W1-wu4-rk W1-wu4-pro W1-nopull-rk W1-nopull-pro
echo "chain1 done $(date +%H:%M:%S)"
