#!/usr/bin/env bash
# Lane A184: one headless automation run (-Multiprocess), its editor pid written to pids.txt; the log to Game/logs/a200/<name>.log.
#   bash ue.sh "<filter>" <name> [ENV=value ...]
cd "$(dirname "$0")/../../../.."
filter="$1"; name="$2"; shift 2
for e in "$@"; do export "$e"; done
UE="C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
mkdir -p Game/logs/a200
"$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests $filter; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$(pwd -W)/Game/logs/a200/$name.log" > /dev/null 2>&1 &
pid=$!
echo "$(date +%H:%M:%S) ue $name pid $pid (sh $$)" >> tools/perf/results/a200/pids.txt
wait $pid
echo "exit $? $(grep -c 'Result={Success}' Game/logs/a200/$name.log) success $(grep -c 'Result={Fail' Game/logs/a200/$name.log) fail"
