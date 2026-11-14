#!/bin/sh
# lane A194: copy of a185, which is a copy of a182/law3.sh. Bar.Curve.Law3 at the levels of $2 (JAM_LAW3_TUNES "level:tune|..."); log $1 under Game/logs/a207/; the editor's pid in Game/logs/a207/$1.pid
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a207
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
env -u JAM_PLAYTESTER_CHECK JAM_LAW3_TUNES="$2" "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.Curve.Law3; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a207/$1.log" > /dev/null 2>&1 &
echo $! > "Game/logs/a207/$1.pid"
wait $!
grep -o "LAW3T.*" "Game/logs/a207/$1.log" | sed 's/ | defender presses.*//' > "Game/logs/a207/$1.txt"
