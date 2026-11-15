#!/bin/sh
# lane A216: the neutral proof. B = main 8cb7e104 as built in Game/logs/a216/shadow; N = this tree with every key of the track neutral. 100 a cell, six cells; one worker each (two processes).
cd "$(dirname "$0")"
echo "chain-proof $$" >> out/pids.txt
SH="$(cd ../../../.. && pwd -W)/Game/logs/a216/shadow/ThirdJam.uproject"
SET="stick.pickup.arc_m=0;stick.pickup.body=false;stick.faceoff.centre_gap_m=0;stick.faceoff.winner_shield_frames=0;stick.faceoff.no_hit_frames=0;puck.net.own_wall=false;puck.net.mouth_only=false"
rm -f raw/B-*.jsonl raw/N-*.jsonl out/proof.txt
for lv in pro as rk; do case $lv in pro) cls=regular; lvl=pro;; as) cls=good; lvl=all_star;; rk) cls=novice; lvl=rookie;; esac
  for rink in regular large; do r=reg; [ $rink = large ] && r=large
    JAM_UPROJECT="$SH" node curve.mjs B-$lv-$r $cls $lvl $rink 100 "" --workers 1 --check closing > /dev/null &
    node curve.mjs N-$lv-$r $cls $lvl $rink 100 "" --workers 1 --check closing --set "$SET" > /dev/null &
    wait
    node same.mjs B-$lv-$r N-$lv-$r >> out/proof.txt
  done; done
echo done > out/proof.done
