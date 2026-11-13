#!/usr/bin/env bash
# Lane A58: l2_night and l1_day again while no other game window is open (their first runs shared the GPU with another lane's game).
cd "$(dirname "$0")/../.."
EXE=Game/Packaged/A58-dev/Windows/ThirdJam/Binaries/Win64/ThirdJam.exe
for t in l2_night l1_day; do powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/match.ps1 -Exe $EXE -Trace -Tier $t -Name ${t}_quiet -NoMemreport; done
echo ALL_DONE
