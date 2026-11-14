#!/bin/sh
# lane A206: Bar.Brain.Interest with JAM_BRAIN_INTEREST=$2 (the ladder's instrument: JAM_PLAYTESTER_CHECK=closing); log Game/logs/a206/$1.log, editor pid in $1.pid
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a206
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
JAM_PLAYTESTER_CHECK=closing JAM_BRAIN_INTEREST="$2" "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.Brain.Interest; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a206/$1.log" > /dev/null 2>&1 &
echo $! > "Game/logs/a206/$1.pid"
wait $!
grep -o "INTEREST \(cell\|idle-by-job\).*" "Game/logs/a206/$1.log" | sed 's/\n.*//' > "Game/logs/a206/$1.rows"
