#!/bin/sh
# lane A208 FACEOFF READY: (a) the gate, then law 3 (default / all neutral); (b) the faceoff strips rookie+pro rule/neutral (one editor each, in turn), then N6 neutral proof vs B1 (2 workers)
cd "$(dirname "$0")"
echo "chain-fo $$" >> out/pids.txt
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
G="$(cd ../../../.. && pwd -W)"
( sh jam.sh gate38 Jam > out/gate38.out 2>&1; JAM_LAW3_AT="8,9.5" sh law3.sh law3a38 "pro:-|all_star:-"; echo a > out/fo.a ) &
( for lv in rookie pro; do pl=novice; [ $lv = pro ] && pl=regular; for arm in rule neutral; do
    JAM_POSS="level=$lv;player=$pl;seeds=5;arm=$arm" "$UE" "$G/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.Possession.FaceoffStrip; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$G/Game/logs/a208/strip-$lv-$arm.log" > /dev/null 2>&1 &
    echo "strip-$lv-$arm $!" >> out/pids.txt; wait $!
    grep -o "POSSTRIP.*" "$G/Game/logs/a208/strip-$lv-$arm.log" >> out/strips.txt
  done; done
  rm -f raw/N6-*.jsonl
  for lv in pro as rk; do
    case $lv in pro) cls=regular; lvl=pro;; as) cls=good; lvl=all_star;; rk) cls=novice; lvl=rookie;; esac
    for rink in regular large; do r=reg; [ $rink = large ] && r=large
      node curve.mjs N6-$lv-$r $cls $lvl $rink 100 "" --workers 2 --check closing --set "stick.pickup.arc_m=0;stick.pickup.body=false;stick.faceoff.centre_gap_m=0;stick.faceoff.winner_shield_frames=0;stick.faceoff.no_hit_frames=0" > /dev/null
      node same.mjs B1-$lv-$r N6-$lv-$r >> out/proofN6.txt
    done; done
  echo b > out/fo.b ) &
wait
