#!/bin/sh
# lane A209: copy of a202/cells.sh on a209/curve.mjs; EXTRA="--seam ... --hand ..." passes options to every cell. Usage: cells.sh <prefix> <n> [tune] [levels]
cd "$(dirname "$0")"
mkdir -p out
P=$1; N=$2; T=${3:-}; L=${4:-"rk pro as"}
echo "$(date +%H:%M:%S) chain-$P sh $$" >> out/pids.txt
for lv in $L; do
  case $lv in pro) cls=regular; lvl=pro;; as) cls=good; lvl=all_star;; rk) cls=novice; lvl=rookie;; esac
  for rink in regular large; do
    r=reg; [ $rink = large ] && r=large
    node curve.mjs $P-$lv-$r $cls $lvl $rink $N "$T" --workers ${W:-2} --check closing $EXTRA > /dev/null
  done
done
echo "chain-$P done"
