#!/bin/sh
# lane A209: copy of a202/law3.sh (logs under Game/logs/a215; EXTRA_ENV="JAM_TEST_A151=..." for an arm). Bar.Curve.Law3 at the levels of $2 (JAM_LAW3_TUNES "level:tune|..."); log $1 under Game/logs/a215/; the editor's pid in Game/logs/a215/$1.pid
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a215
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
env -u JAM_PLAYTESTER_CHECK $EXTRA_ENV JAM_LAW3_TUNES="$2" "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.Curve.Law3; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a215/$1.log" > /dev/null 2>&1 &
echo $! > "Game/logs/a215/$1.pid"
wait $!
grep -o "LAW3T.*" "Game/logs/a215/$1.log" | sed 's/ | defender presses.*//' > "Game/logs/a215/$1.txt"
