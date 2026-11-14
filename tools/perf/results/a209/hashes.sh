#!/bin/sh
# lane A209: the default match's hashes, 100 seeds a level and rink, NO measuring variable unless EXTRA is set (the neutral proof). Usage: hashes.sh <prefix>
cd "$(dirname "$0")"
mkdir -p out
P=$1
echo "$(date +%H:%M:%S) chain-hash-$P sh $$" >> out/pids.txt
for lv in rk pro as; do
  case $lv in pro) cls=regular; lvl=pro;; as) cls=good; lvl=all_star;; rk) cls=novice; lvl=rookie;; esac
  for rink in regular large; do
    r=reg; [ $rink = large ] && r=large
    env -u JAM_PLAYTESTER_CHECK node curve.mjs $P-$lv-$r $cls $lvl $rink 100 "" --workers 3 $EXTRA > /dev/null
  done
done
echo "chain-hash-$P done $(date +%H:%M:%S)"
