#!/usr/bin/env bash
# Lane A58: the two tier runs repeated without an Insights export running beside them (the first p3_clay / p4_broadcast runs
# overlapped my own trace analysis on the CPU), then the ten-rebuild leak run (no trace, 20 s match, jam.livecut x10, memreport after each).
cd "$(dirname "$0")/../.."
EXE=Game/Packaged/A58-dev/Windows/ThirdJam/Binaries/Win64/ThirdJam.exe
for t in p3_clay p4_broadcast; do powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/match.ps1 -Exe $EXE -Trace -Tier $t -Name $t; done
powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/match.ps1 -Exe $EXE -Tier l2_night -Name leak10 -MatchSeconds 20 -LiveCuts 10
echo ALL_DONE
