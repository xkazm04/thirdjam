#!/usr/bin/env bash
# Lane A58: the Insights exports behind PROFILING.md, one Insights at a time, after the games are closed.
# Per run: game-thread and GPU timer statistics over the CSV window (csvprofile start..stop, trace time = driver time - 0.25 s),
# worker-thread statistics (animation evaluation, Niagara), and the hitch list with the top stack of the four worst frames.
cd "$(dirname "$0")/../.."
X() { powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/insights-export.ps1 -Trace "$1" -Cmd "$2" | tail -1; }
for d in ${RUNS:-l2_night_quiet p3_clay p4_broadcast l1_day_quiet}; do
  ev=Game/Packaged/perf-out/$d/events.log
  s=$(grep -o "t=[0-9.]* console 'csvprofile start" $ev | head -1 | sed 's/t=\([0-9.]*\).*/\1/'); e=$(grep -o "t=[0-9.]* console 'csvprofile stop" $ev | head -1 | sed 's/t=\([0-9.]*\).*/\1/')
  s=$(awk "BEGIN{print $s-0.25}"); e=$(awk "BEGIN{print $e-0.25}")
  tr=Game/Packaged/perf-out/$d/trace.utrace
  echo "== $d window $s..$e"
  X $tr "TimingInsights.ExportTimerStatistics {out}\gt-match.csv -threads=GameThread -startTime=$s -endTime=$e -sortBy=TotalInclusiveTime -maxTimerCount=500"
  X $tr "TimingInsights.ExportTimerStatistics {out}\gpu-match.csv -threads=GPU0-Graphics0,GPU0-Compute0 -startTime=$s -endTime=$e -sortBy=TotalExclusiveTime -maxTimerCount=300"
  X $tr "TimingInsights.ExportTimerStatistics {out}\rt-match.csv -threads=RenderThread* -startTime=$s -endTime=$e -sortBy=TotalExclusiveTime -maxTimerCount=200"
  X $tr "TimingInsights.ExportTimerStatistics {out}\workers-match.csv -threads=Background*,Foreground* -startTime=$s -endTime=$e -sortBy=TotalInclusiveTime -maxTimerCount=200"
  node tools/perf/hitches.mjs $tr --n 4 --json Game/Packaged/perf-out/$d/hitches.json > Game/Packaged/perf-out/$d/hitches.txt 2>&1
  head -1 Game/Packaged/perf-out/$d/hitches.txt
done
echo ALL_DONE
