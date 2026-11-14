#!/bin/sh
# lane A208: the neutral proof: six cells x 100 with stick.pickup at its neutral value (arc_m 0, body false), against B0 (build 1 = main). Usage: proof.sh <prefix>
cd "$(dirname "$0")"
mkdir -p out
P=${1:-N0}
echo "chain-$P $$" >> out/pids.txt
for lv in pro as rk; do
  case $lv in pro) cls=regular; lvl=pro;; as) cls=good; lvl=all_star;; rk) cls=novice; lvl=rookie;; esac
  for rink in regular large; do
    r=reg; [ $rink = large ] && r=large
    node curve.mjs $P-$lv-$r $cls $lvl $rink 100 "" --workers 2 --check closing --set "stick.pickup.arc_m=0;stick.pickup.body=false" > /dev/null
    [ -z "$NOSAME" ] && node same.mjs B0-$lv-$r $P-$lv-$r
  done
done
echo "chain-$P done"
