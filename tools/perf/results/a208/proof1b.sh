#!/bin/sh
# RETIRED 23:40 (lane A208): ran on the main checkout; never use.
exit 1
# lane A208 READY 1b: B1 = main d0d132bf's binaries (JAM_UPROJECT, the main checkout), 100 a cell; N4 = this tree with stick.pickup neutral; then same.mjs
cd "$(dirname "$0")"
echo "chain-proof1b $$" >> out/pids.txt
for lv in pro as rk; do
  case $lv in pro) cls=regular; lvl=pro;; as) cls=good; lvl=all_star;; rk) cls=novice; lvl=rookie;; esac
  for rink in regular large; do
    r=reg; [ $rink = large ] && r=large
    JAM_UPROJECT='C:/Users/kazda/kiro/thirdjam/Game/ThirdJam.uproject' node curve.mjs B1-$lv-$r $cls $lvl $rink 100 "" --workers 2 --check closing > /dev/null
  done
done
NOSAME=1 sh proof.sh N4
for c in pro-reg pro-large as-reg as-large rk-reg rk-large; do node same.mjs B1-$c N4-$c; done
echo proof1b-done
