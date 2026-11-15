#!/bin/sh
# lane A218: editor Development build of this worktree; log Game/logs/a218/$1.log, pid of the powershell in Game/logs/a218/$1.pid
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a218
powershell -NoProfile -ExecutionPolicy Bypass -File tools/run/build.ps1 -Project "$(pwd -W)/Game" ${2:+-Config $2} -TimeoutSec 3000 > "Game/logs/a218/$1.log" 2>&1 &
echo $! > "Game/logs/a218/$1.pid"
wait $!
echo "exit $?" >> "Game/logs/a218/$1.log"
