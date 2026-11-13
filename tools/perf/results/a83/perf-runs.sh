#!/bin/bash
# Lane A83: 3 v 3 and 5 v 5 profiled bot matches on the A83 Development package, one game at a time (match.ps1, remote console port 4683).
cd "$(dirname "$0")/../../../.."
EXE='Game\Packaged\A83-dev\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe'
for run in l2_night l2_night_5v5 p3_clay p3_clay_5v5; do
  powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/match.ps1 -Exe "$EXE" -Tier $run -Name a83-$run -Trace -NoMemreport -Port 4683 > tools/perf/results/a83/match-$run.log 2>&1
  echo "$run exit $?"
done
