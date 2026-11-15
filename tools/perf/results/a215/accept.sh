#!/bin/sh
# lane A215: one windowed acceptance run of a package of this worktree, under the machine's window lock (mkdir before, rmdir after, pass or fail)
cd "$(dirname "$0")/../../../.."
LOCK=/c/Users/kazda/AppData/Local/Temp/tj/tj-window.lock
mkdir -p /c/Users/kazda/AppData/Local/Temp/tj
until mkdir "$LOCK" 2>/dev/null; do echo "$(date +%H:%M:%S) lock held, waiting"; sleep 15; done
echo "$(date +%H:%M:%S) accept $2 took the window lock (sh $$)" >> Game/logs/a215/pids.txt
powershell -NoProfile -ExecutionPolicy Bypass -File tools/qa/accept/accept.ps1 -Package "$(pwd -W)/Game/Packaged/$1" -Name "$2" -Port 4809 > "Game/logs/a215/$2.txt" 2>&1 &
p=$!
echo "$(date +%H:%M:%S) accept $2 powershell pid $p" >> Game/logs/a215/pids.txt
wait $p; rc=$?
rmdir "$LOCK"
echo "$(date +%H:%M:%S) accept $2 exit $rc, lock removed" | tee -a Game/logs/a215/pids.txt
grep -E "^(PASS|FAIL) step" "Game/logs/a215/$2.txt"
