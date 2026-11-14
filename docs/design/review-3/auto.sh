#!/usr/bin/env bash
# Lane A169 (REVIEW-3): automation, headless. $1 = a name, $2 = the test filter; env passes through (JAM_SWEEP_FILE, JAM_CARRY_TRACE). Run from the worktree's root.
R=docs/design/review-3/raw
UE="C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
W="$(pwd -W)"
"$UE" "$W/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests $2; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$W/$R/$1.log" > $R/$1.out 2>&1
echo "exit $?" > $R/$1.done
