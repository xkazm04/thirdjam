#!/bin/sh
# lane A216: STICK READY 3 measures on build 6: Bar.Hard.PuckEscape 480 (two processes), then Bar.A216.OwnGoal wall on / off (two processes)
cd "$(dirname "$0")/../../../.."
echo "chain-after $$" >> tools/perf/results/a216/out/pids.txt
G="$(pwd -W)"; UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
for h in 1 6; do env -u JAM_HARD_TRACE JAM_HARD_SEEDS=5 JAM_HARD_FIRST=$h "$UE" "$G/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.Hard.PuckEscape; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$G/Game/logs/a216/escA-$h.log" > /dev/null 2>&1 &
  echo "escA-$h $!" >> tools/perf/results/a216/out/pids.txt; done; wait
for arm in on off; do w=""; [ $arm = off ] && w="wall=0"; JAM_A216="from=1;to=40;$w" "$UE" "$G/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.A216.OwnGoal; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$G/Game/logs/a216/own-$arm.log" > /dev/null 2>&1 &
  echo "own-$arm $!" >> tools/perf/results/a216/out/pids.txt; done; wait
echo done > tools/perf/results/a216/out/after.done
