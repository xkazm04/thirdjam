#!/bin/sh
# lane A216: law 3 on the merged tree, then the faceoff strips rookie/pro x rule/neutral (one editor at a time)
cd "$(dirname "$0")"
echo "chain2 $$" >> out/pids.txt
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
G="$(cd ../../../.. && pwd -W)"
JAM_LAW3_AT="8,8.5,9,9.5" sh law3.sh law3m "pro:-|all_star:-"
for lv in rookie pro; do pl=novice; [ $lv = pro ] && pl=regular; for arm in rule neutral; do
  JAM_POSS="level=$lv;player=$pl;seeds=5;arm=$arm" "$UE" "$G/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.Possession.FaceoffStrip; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$G/Game/logs/a216/strip-$lv-$arm.log" > /dev/null 2>&1 &
  echo "strip-$lv-$arm $!" >> out/pids.txt; wait $!
  grep -o "POSSTRIP.*" "$G/Game/logs/a216/strip-$lv-$arm.log" >> out/strips.txt
done; done
echo done > out/chain2.done
