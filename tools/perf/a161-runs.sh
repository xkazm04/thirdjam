#!/usr/bin/env bash
# Lane A161: the measurement batch on the merged tree's Development package, off screen (-Offscreen), port 4761, PLAY NOW with no cut
# (hockey.regular = p3_clay, five a side, the operator's match). One game at a time.
#   bash tools/perf/a161-runs.sh <set> ;  sets: stands | shadow | goallight | first
set -u
wt="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$wt"
exe="Game/Packaged/A161/Windows/ThirdJam/Binaries/Win64/ThirdJam.exe"
old="Game/Packaged/A150c/Windows/ThirdJam/Binaries/Win64/ThirdJam.exe"
bot() { powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/bot.ps1 -Port 4761 -Offscreen -StartPath play -NoCut "$@" | tail -2; }
case "$1" in
  stands)    for i in 1 2 3; do first=hide; [ $((i % 2)) -eq 0 ] && first=show
               bot -Exe "$exe" -Tier p3_clay_5v5 -Name "a161-st-$i" -Dp 'jam.PreloadRig=0,jam.WarmScene=0' -PlaySeconds 150 -ToggleMeshes 'SM_(Fan|FanRow|Stand)_' -ToggleSec 30 -ToggleFirst $first; done ;;
  shadow)    bot -Exe "$exe" -Tier p3_clay_5v5 -Name a161-shadow-1 -PlaySeconds 150 -ToggleSec 20 \
               -ToggleHideCmd 'r.Shadow.Virtual.ResolutionLodBiasLocal 1' -ToggleShowCmd 'r.Shadow.Virtual.ResolutionLodBiasLocal 0' ;;
  goallight) bot -Exe "$exe" -Tier p3_clay_5v5 -Name a161-goallight-1 -PlaySeconds 180 -ToggleSec 15 \
               -ToggleHideCmd 'jam.rink.react.light 0' -ToggleShowCmd 'jam.rink.react.light 6000' ;;
  first)     for i in 1 2 3; do
               if [ $((i % 2)) -eq 1 ]; then bot -Exe "$old" -Tier p3_clay_5v5 -Name "a161-first-a150c-$i" -PlaySeconds 0; bot -Exe "$exe" -Tier p3_clay_5v5 -Name "a161-first-a161-$i" -PlaySeconds 0
               else bot -Exe "$exe" -Tier p3_clay_5v5 -Name "a161-first-a161-$i" -PlaySeconds 0; bot -Exe "$old" -Tier p3_clay_5v5 -Name "a161-first-a150c-$i" -PlaySeconds 0; fi; done ;;
esac
