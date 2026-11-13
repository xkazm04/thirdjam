#!/bin/bash
# Lane A83: a 5 v 5 match on the A83 package (p3_clay cut, ten figures) and a PrintWindow still every 2 s of play (shot.ps1).
cd "$(dirname "$0")/../../../.."
EXE='Game\Packaged\A83-dev\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe'
OUT=tools/perf/out/a83-still
rm -rf $OUT; mkdir -p $OUT/shots
powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/match.ps1 -Exe "$EXE" -Tier p3_clay_5v5 -Name a83-still -MatchSeconds 75 -NoMemreport -Port 4683 > tools/perf/results/a83/match-still.log 2>&1 &
for i in $(seq 1 120); do grep -q "csvprofile start" $OUT/events.log 2>/dev/null && break; sleep 1; done
PID=$(grep -o "start pid [0-9]*" $OUT/events.log | grep -o "[0-9]*$")
for i in $(seq -w 1 36); do
  powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/shot.ps1 -ProcessId $PID -Out "$(cygpath -w $PWD/$OUT/shots/s$i.png)" | tail -n 1
  sleep 1.5
done
wait
