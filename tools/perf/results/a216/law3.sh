#!/bin/sh
# lane A216 (copy of a202): copy of a182/law3.sh. Bar.Curve.Law3 at the levels of $2 (JAM_LAW3_TUNES "level:tune|..."); log $1 under Game/logs/a216/; the editor's pid in Game/logs/a216/$1.pid
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a216
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
env -u JAM_PLAYTESTER_CHECK JAM_LAW3_TUNES="$2" "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.Curve.Law3; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a216/$1.log" > /dev/null 2>&1 &
echo $! > "Game/logs/a216/$1.pid"
wait $!
grep -o "LAW3T.*" "Game/logs/a216/$1.log" | sed 's/ | defender presses.*//' > "Game/logs/a216/$1.txt"
