#!/usr/bin/env bash
# Lane A58: the four profiled bot matches, one game at a time. Default tier first (with the jam.livecut leak check), then the others.
cd "$(dirname "$0")/../.."
EXE=Game/Packaged/A58-dev/Windows/ThirdJam/Binaries/Win64/ThirdJam.exe
run() { powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/match.ps1 -Exe $EXE -Trace "$@"; }
run -Tier l2_night -Name l2_night -LiveCuts 3
for t in ${TIERS:-p3_clay p4_broadcast l1_day}; do run -Tier $t -Name $t; done
echo ALL_DONE
