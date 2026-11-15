#!/bin/sh
# lane A202: Bar.Brain.Watch with JAM_BRAIN_WATCH=$2 (the ladder's instrument: JAM_PLAYTESTER_CHECK=closing); log Game/logs/a206/$1.log, pid in $1.pid
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a206
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
JAM_PLAYTESTER_CHECK=closing JAM_BRAIN_WATCH="$2" "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.Brain.Watch; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a206/$1.log" > /dev/null 2>&1 &
echo $! > "Game/logs/a206/$1.pid"
wait $!
grep -o "WATCH row.*" "Game/logs/a206/$1.log" > "Game/logs/a206/$1.rows"
