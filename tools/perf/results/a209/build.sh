#!/bin/sh
# lane A209: one editor build of this worktree (-WaitMutex through build.ps1's UBT); the log in Game/logs/a209/$1.log; pids in tools/perf/results/a209/pids.txt
cd "$(dirname "$0")/../../../.."
echo "$(date +%H:%M:%S) build $1 sh $$" >> tools/perf/results/a209/pids.txt
powershell -NoProfile -ExecutionPolicy Bypass -File tools/run/build.ps1 -Project "$(pwd -W)/Game" -TimeoutSec 3000 > "Game/logs/a209/$1.log" 2>&1 &
p=$!
echo "$(date +%H:%M:%S) build $1 powershell pid $p" >> tools/perf/results/a209/pids.txt
wait $p
echo "build $1 exit $?" | tee -a tools/perf/results/a209/pids.txt
tail -5 "Game/logs/a209/$1.log"
