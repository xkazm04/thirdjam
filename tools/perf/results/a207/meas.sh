#!/bin/sh
# lane A207: Bar.Pass.Measure with JAM_PASS_MEASURE=$2; log Game/logs/a207/$1.log, the editor's pid in $1.pid; summary rows in Game/logs/a207/$1.txt
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a207
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
env -u JAM_PLAYTESTER_CHECK JAM_PASS_MEASURE="$2" "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests ${3:-Bar.Pass.Measure}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a207/$1.log" > /dev/null 2>&1 &
echo $! > "Game/logs/a207/$1.pid"
wait $!
grep -o "\(PASSSUM|SHOTSUM|SHOTFATE|PASSMATCHES|PASSTHROUGH|SHOTSUM|PASSMATCHES|PASSTHROUGH|STAGED[A-Z]*|SHOTSTAGE)\|SHOTSUM\|PASSMATCHES\|STAGED[A-Z]*\|SHOTSTAGE\).*" "Game/logs/a207/$1.log" | sed 's/\x27, file.*//; s/", file.*//' > "Game/logs/a207/$1.txt"
grep -E "Test Completed. Result" "Game/logs/a207/$1.log" | sed 's/.*Result=/Result=/' >> "Game/logs/a207/$1.txt"
