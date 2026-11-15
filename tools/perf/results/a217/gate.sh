#!/bin/sh
# lane A217 LADDER SET 3's gate on the built picks: the full Jam filter in a cleared environment (no JAM_* variable) beside the Shipping compile.
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a217
echo "gate $$ $(date +%H:%M)" >> Game/logs/a217/pids.txt
W=$(cygpath -w "$PWD")
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
CLEAN=$(env | sed -n 's/^\(JAM_[A-Za-z0-9_]*\)=.*/-u \1/p' | tr '\n' ' ')
env $CLEAN "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Jam; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a217/jam-set3e.log" > /dev/null 2>&1 &
echo "gate jam pid $! $(date +%H:%M)" >> Game/logs/a217/pids.txt
powershell -NoProfile -ExecutionPolicy Bypass -File "$W\tools\run\build.ps1" -Project "$W\Game\ThirdJam.uproject" -Config Shipping -TimeoutSec 5400 > Game/logs/a217/shipping.out 2>&1
wait
grep -E "Test Completed\. Result=\{(Fail|Success)" Game/logs/a217/jam-set3e.log | sed 's/.*Result={\([A-Za-z]*\)} Name={\([^}]*\)}.*/\1 \2/' > Game/logs/a217/jam-set3e.txt
echo "gate done $(date +%H:%M): $(grep -c '^Success' Game/logs/a217/jam-set3e.txt) success, $(grep -c '^Fail' Game/logs/a217/jam-set3e.txt) fail; shipping $(grep -E '^(PASS|FAIL)' Game/logs/a217/shipping.out)" >> Game/logs/a217/pids.txt
