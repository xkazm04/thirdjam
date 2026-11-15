#!/bin/sh
# lane A216: one editor running test filter $2 with env JAM_A216=$3; log Game/logs/a216/$1.log; pid in Game/logs/a216/$1.pid
cd "$(dirname "$0")/../../../.."
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
for v in $(env | grep -o '^JAM_[A-Z0-9_]*'); do unset "$v"; done
JAM_A216="$3" "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests $2; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a216/$1.log" > /dev/null 2>&1 &
echo $! > "Game/logs/a216/$1.pid"
wait $!
grep -E "Result=\{(Fail|Success)" "Game/logs/a216/$1.log" | sed 's/.*Result={\([A-Za-z]*\)} Name={\([^}]*\)}.*/\1 \2/'
grep -oE "(A216[A-Z]*|Error: ).*" "Game/logs/a216/$1.log" | grep -v "^Error: Test Completed" | head -${4:-200}
