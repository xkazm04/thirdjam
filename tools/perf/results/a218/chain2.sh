#!/bin/sh
# lane A218: the proof chain on the built tree. (1) gate (full Jam, clean env) beside law 3 (pro / all_star at 8 / 8.5 / 9 / 9.5 m); (2) the escape scan
# (Bar.Hard.PuckEscape, 3 seeds a case) beside the Shipping compile. Two heavy processes at a time. Pids: Game/logs/a218/*.pid; this shell's in chain2.sh.pid.
cd "$(dirname "$0")"
G="$(cd ../../../.. && pwd -W)"
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
echo $$ > "$G/Game/logs/a218/chain2.sh.pid"
sh jam.sh gate2 Jam > "$G/Game/logs/a218/gate2.sum" 2>&1 &
JAM_LAW3_AT="8,8.5,9,9.5" sh law3.sh law3b "pro:-|all_star:-" &
wait
env -u JAM_HARD_TRACE JAM_HARD_SEEDS=3 JAM_HARD_FIRST=1 "$UE" "$G/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests Bar.Hard.PuckEscape; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$G/Game/logs/a218/esc2.log" > /dev/null 2>&1 &
echo $! > "$G/Game/logs/a218/esc2.pid"
sh build.sh ship2 Shipping &
wait
echo done > "$G/Game/logs/a218/chain2.done"
