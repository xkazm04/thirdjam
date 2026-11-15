#!/bin/sh
# lane A208 READY 2 on build 43: gate + law 3 (one editor), N7 neutral proof (all my keys neutral) vs B1 (2 workers)
cd "$(dirname "$0")"
echo "chain-r2 $$" >> out/pids.txt
( sh jam.sh gate43 Jam > out/gate43.out 2>&1; JAM_LAW3_AT="8,9.5" sh law3.sh law3a43 "pro:-|all_star:-"; echo a > out/r2.a ) &
( rm -f raw/N7-*.jsonl; for lv in pro as rk; do case $lv in pro) cls=regular; lvl=pro;; as) cls=good; lvl=all_star;; rk) cls=novice; lvl=rookie;; esac
    for rink in regular large; do r=reg; [ $rink = large ] && r=large
      node curve.mjs N7-$lv-$r $cls $lvl $rink 100 "" --workers 2 --check closing --set "stick.pickup.arc_m=0;stick.pickup.body=false;stick.faceoff.centre_gap_m=0;stick.faceoff.winner_shield_frames=0;stick.faceoff.no_hit_frames=0;puck.net.own_wall=false;puck.net.mouth_only=false" > /dev/null
      node same.mjs B1-$lv-$r N7-$lv-$r >> out/proofN7.txt; done; done; echo b > out/r2.b ) &
wait
