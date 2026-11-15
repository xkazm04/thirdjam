#!/bin/sh
# lane A216: (1) B0 = main 8cb7e104 (shadow) with stick.pickup off vs N (this tree, every key of the track neutral; rows of proof.sh);
# (2) N2 = this tree with only wave 2's keys neutral (faceoff, net) vs B (main as it ships; rows of proof.sh). One worker each.
cd "$(dirname "$0")"
echo "chain-proof2 $$" >> out/pids.txt
SH="$(cd ../../../.. && pwd -W)/Game/logs/a216/shadow/ThirdJam.uproject"
SET2="stick.faceoff.centre_gap_m=0;stick.faceoff.winner_shield_frames=0;stick.faceoff.no_hit_frames=0;puck.net.own_wall=false;puck.net.mouth_only=false"
rm -f raw/B0-*.jsonl raw/N2-*.jsonl out/proof2.txt
for lv in pro as rk; do case $lv in pro) cls=regular; lvl=pro;; as) cls=good; lvl=all_star;; rk) cls=novice; lvl=rookie;; esac
  for rink in regular large; do r=reg; [ $rink = large ] && r=large
    JAM_UPROJECT="$SH" node curve.mjs B0-$lv-$r $cls $lvl $rink 100 "" --workers 1 --check closing --set "stick.pickup.arc_m=0;stick.pickup.body=false" > /dev/null &
    node curve.mjs N2-$lv-$r $cls $lvl $rink 100 "" --workers 1 --check closing --set "$SET2" > /dev/null &
    wait
    node same.mjs B0-$lv-$r N-$lv-$r >> out/proof2.txt
    node same.mjs B-$lv-$r N2-$lv-$r >> out/proof2.txt
  done; done
echo done > out/proof2.done
