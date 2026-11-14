#!/bin/sh
# lane A185: the gate's automation filter ($2, default Jam) in a clean environment (no JAM_* measuring variable reaches it); log Game/logs/a202/$1.log, pid in $1.pid
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a202
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
for v in $(env | grep -o '^JAM_[A-Z0-9_]*'); do unset "$v"; done
"$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests ${2:-Jam}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a202/$1.log" > /dev/null 2>&1 &
echo $! > "Game/logs/a202/$1.pid"
wait $!
grep -E "Test Completed\. Result=\{(Fail|Success)" "Game/logs/a202/$1.log" | sed 's/.*Result={\([A-Za-z]*\)} Name={\([^}]*\)}.*/\1 \2/' > "Game/logs/a202/$1.results"
cut -d" " -f1 "Game/logs/a202/$1.results" | sort | uniq -c; grep "^Fail" "Game/logs/a202/$1.results"
