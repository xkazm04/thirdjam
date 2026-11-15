#!/bin/sh
# lane A221: the full Jam filter in a cleared environment (no JAM_* variable); log Game/logs/a221/$1.log, results $1.txt
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a221
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
CLEAN=$(env | sed -n 's/^\(JAM_[A-Za-z0-9_]*\)=.*/-u \1/p' | tr '\n' ' ')
env $CLEAN "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests ${2:-Jam}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a221/$1.log" > /dev/null 2>&1
grep -E "Test Completed\. Result=\{(Fail|Success)" Game/logs/a221/$1.log | sed 's/.*Result={\([A-Za-z]*\)} Name={\([^}]*\)}.*/\1 \2/' > Game/logs/a221/$1.txt
echo "$1 done $(date +%H:%M): $(grep -c '^Success' Game/logs/a221/$1.txt) success, $(grep -c '^Fail' Game/logs/a221/$1.txt) fail" >> Game/logs/a221/pids.txt
