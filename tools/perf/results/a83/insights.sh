#!/bin/bash
# Lane A83: Insights timer statistics (game thread) of each a83 trace, one at a time.
cd "$(dirname "$0")/../../../.."
for r in l2_night l2_night_5v5 p3_clay p3_clay_5v5; do
  powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/insights-export.ps1 -Trace tools/perf/out/a83-$r/trace.utrace -Cmd "TimingInsights.ExportTimerStatistics {out}\gt-$r.csv -threads=GameThread -sortBy=TotalInclusiveTime" > tools/perf/results/a83/insights-$r.log 2>&1
  echo "$r exit $?"
done
