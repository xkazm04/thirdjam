#!/usr/bin/env bash
# Lane A175: an Insights trace of the clay 5 v 5 match (ten BP_SkaterVisual_TB1_P3 figures), off screen, then the game thread's timer statistics
# exported headless and the figure Blueprint's functions picked out (UpdateMotion per call = per figure per tick).
#   bash tools/perf/a175-trace.sh [exe] [seconds=25]
set -u
wt="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$wt"
exe="${1:-Game/Packaged/A175-cold-1/ThirdJam/Binaries/Win64/ThirdJam.exe}"; secs="${2:-25}"
name=a175-trace
MSYS_NO_PATHCONV=1 powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/bot.ps1 -Exe "$exe" -Tier p3_clay_5v5 -Name $name -Port 4775 \
  -Offscreen -StartPath play -NoCut -PlaySeconds "$secs" -Trace -TraceChannels 'cpu,frame,bookmark' | tail -1
ls -la "tools/perf/out/$name/trace.utrace"
MSYS_NO_PATHCONV=1 powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/insights-export.ps1 -Trace "tools/perf/out/$name/trace.utrace" \
  -Cmd "TimingInsights.ExportTimerStatistics {out}\timers-gt.csv -threads=GameThread -sortBy=TotalInclusiveTime"
f="tools/perf/out/$name/insights/timers-gt.csv"
head -1 "$f"; grep -i -E "UpdateMotion|SkaterVisual|ReceiveTick|Frame,|GameEngineTick|Blueprint" "$f" | head -30
