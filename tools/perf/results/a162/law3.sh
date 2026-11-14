#!/bin/sh
# lane A133: Bar.Curve.Law3 under candidate tunes (JAM_LAW3_TUNES "level:tune|..."); log $1 under Game/logs/a162/
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a162
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
JAM_LAW3_TUNES="$2" "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.Curve.Law3; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a162/$1.log" > /dev/null 2>&1
grep -o "LAW3T.*" "Game/logs/a162/$1.log" | sed 's/ | defender presses.*//' > "Game/logs/a162/$1.txt"
