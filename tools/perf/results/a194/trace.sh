#!/bin/sh
# lane A185: Bar.Movement.CarryTrace (100 matches) with JAM_CARRY_TRACE=$2; log Game/logs/a194/$1.log, pid in $1.pid
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a194
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
env -u JAM_PLAYTESTER_CHECK JAM_CARRY_TRACE="$2" "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.Movement.CarryTrace; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a194/$1.log" > /dev/null 2>&1 &
echo $! > "Game/logs/a194/$1.pid"
wait $!
