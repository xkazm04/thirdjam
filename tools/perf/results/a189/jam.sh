#!/bin/sh
# lane A133: the full Jam filter (the gate's invocation), log $1 under Game/logs/a189/
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a189
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
env -u JAM_PLAYTESTER_CHECK -u JAM_PLAYTESTER_POKE -u JAM_PLAYTESTER_STICK -u JAM_LAW3_TUNES "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests ${2:-Jam}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a189/$1.log" > /dev/null 2>&1
grep -E "Test Completed\. Result=\{(Fail|Success)" "Game/logs/a189/$1.log" | sed 's/.*Result={\([A-Za-z]*\)} Name={\([^}]*\)}.*/\1 \2/' | sort | uniq -c | awk '{print $2}' | sort | uniq -c
