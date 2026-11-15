#!/bin/sh
# lane A215: a Development package of this worktree under Game/Packaged/a215-$1; the log in Game/logs/a215/pkg-$1.log; pids in Game/logs/a215/pids.txt
cd "$(dirname "$0")/../../../.."
echo "$(date +%H:%M:%S) package $1 sh $$" >> Game/logs/a215/pids.txt
powershell -NoProfile -ExecutionPolicy Bypass -File tools/run/package.ps1 -Project "$(pwd -W)/Game" -Out "$(pwd -W)/Game/Packaged/a215-$1" -Config Development -ExtraArgs "-ubtargs=-WaitMutex" > "Game/logs/a215/pkg-$1.log" 2>&1 &
p=$!
echo "$(date +%H:%M:%S) package $1 powershell pid $p" >> Game/logs/a215/pids.txt
wait $p
echo "package $1 exit $?" | tee -a Game/logs/a215/pids.txt
tail -5 "Game/logs/a215/pkg-$1.log"
