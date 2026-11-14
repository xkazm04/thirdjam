#!/bin/sh
# lane A133: the full Jam filter (the gate's invocation), log $1 under Game/logs/a162/
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a162
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
"$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests ${2:-Jam}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a162/$1.log" > /dev/null 2>&1
grep -E "Test Completed\. Result=\{(Fail|Success)" "Game/logs/a162/$1.log" | sed 's/.*Result={\([A-Za-z]*\)} Name={\([^}]*\)}.*/\1 \2/' | sort | uniq -c | awk '{print $2}' | sort | uniq -c
