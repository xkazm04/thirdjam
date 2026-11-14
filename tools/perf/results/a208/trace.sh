#!/bin/sh
# lane A208: Bar.Possession.Trace, arms x levels, 100 matches each, one process at a time. Usage: trace.sh <prefix> [arms] [levels]
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a208
P=$1; ARMS=${2:-"neutral rule"}; LV=${3:-"rookie pro"}
echo "chain-trace-$P $$" >> tools/perf/results/a208/out/pids.txt
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
for lv in $LV; do for arm in $ARMS; do
  pl=novice; [ $lv = pro ] && pl=regular; [ $lv = all_star ] && pl=good
  JAM_POSS="level=$lv;player=$pl;seeds=100;from=1;arm=$arm" "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.Possession.Trace; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a208/$P-$lv-$arm.log" > /dev/null 2>&1 &
  echo "trace-$P-$lv-$arm $!" >> tools/perf/results/a208/out/pids.txt
  wait $!
  grep -o "POSTRACE sum.*\|POSTRACE why-not.*" "Game/logs/a208/$P-$lv-$arm.log" >> tools/perf/results/a208/out/$P.trace.txt
done; done
echo "chain-trace-$P done" >> tools/perf/results/a208/out/$P.trace.txt
