#!/bin/sh
# lane A206: copy of a202/cells.sh. Lane A202: six cells (three levels x two rinks, the ladder's instrument) at n seeds with one tune. Usage: cells.sh <prefix> <n> [tune] [levels: default "pro as rk"]
# Labels <prefix>-<cell>; rows in cells.jsonl; child pids in out/pids.txt.
cd "$(dirname "$0")"
mkdir -p out
P=$1; N=$2; T=${3:-}; L=${4:-"pro as rk"}
echo "chain-$P $$" >> out/pids.txt
for lv in $L; do
  case $lv in pro) cls=regular; lvl=pro;; as) cls=good; lvl=all_star;; rk) cls=novice; lvl=rookie;; esac
  for rink in regular large; do
    r=reg; [ $rink = large ] && r=large
    node curve.mjs $P-$lv-$r $cls $lvl $rink $N "$T" --workers ${W:-2} --check closing > /dev/null
  done
done
echo "chain-$P done"
