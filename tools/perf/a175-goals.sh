#!/usr/bin/env bash
# Lane A175: the goal, timed. tools/perf/cuts/perf.p3_clay_goals (the clay 5 v 5 perf cut with 10 m nets, the save by height off, a 1 s goal
# pause) scores many goals; the sim is deterministic (cut + seed + frames = the same state), so every arm sees its goals on the same sim frames and
# the arms pair goal by goal. Arms: jam.rink.react.light 6000 (the loud goal as shipped), 800 (B41's clay candelas), 0 (no light). Off screen, 4775.
#   bash tools/perf/a175-goals.sh [exe] [rounds=2] [seconds=100]
set -u
wt="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$wt"
exe="${1:-Game/Packaged/A175-cold-1/ThirdJam/Binaries/Win64/ThirdJam.exe}"; rounds="${2:-2}"; secs="${3:-100}"
for k in $(seq 1 "$rounds"); do
  arms="6000 800 0"; [ $((k % 2)) -eq 0 ] && arms="0 800 6000"
  for L in $arms; do
    MSYS_NO_PATHCONV=1 powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/bot.ps1 -Exe "$exe" -Tier p3_clay_goals -Name "a175-goal-$L-$k" \
      -Port 4775 -Offscreen -StartPath play -PlaySeconds "$secs" -Dp "jam.rink.react.light=$L" | tail -1
  done
done
