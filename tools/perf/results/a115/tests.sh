#!/bin/sh
# usage: tests.sh <filter> <tag>   headless automation in its own editor process, log Game/logs/a115/ue-<tag>.log
W=/c/Users/kazda/kiro/thirdjam/.claude/worktrees/a105
UE="/c/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
LOG="$(cygpath -w $W/Game/logs/a115/ue-$2.log)"
"$UE" "$(cygpath -w $W/Game/ThirdJam.uproject)" -ExecCmds="Automation RunTests $1; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$LOG" > /dev/null 2>&1
echo "exit $?"
grep -E "Test Completed\. Result=\{" "$W/Game/logs/a115/ue-$2.log" | sed -E 's/.*Result=\{([^}]*)\} Name=\{([^}]*)\}.*/\1 \2/' | sort | uniq -c | sort -rn | awk '{print $2}' | sort | uniq -c
grep -E "Test Completed\. Result=\{Fail" "$W/Game/logs/a115/ue-$2.log" | sed -E 's/.*Name=\{([^}]*)\}.*/FAIL \1/' | head -60
