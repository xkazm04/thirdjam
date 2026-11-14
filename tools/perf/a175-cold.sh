#!/usr/bin/env bash
# Lane A175: the operator's first start on a fresh unzip against his second start. Each round copies the acceptance package of the build he plays
# (Development twin of the Shipping zip; read-only source) into a new folder without its Saved/ (a fresh unzip into a new folder: no user pipeline
# cache, a new path for the driver), then launches it twice through PLAY NOW, no cut, off screen: launch 1 = cold, launch 2 = warm. Both carry
# r.ShaderPipelineCache.SaveUserCache=1, the Shipping default (Development leaves it off), so launch 1's clean exit writes what launch 2 reads.
#   bash tools/perf/a175-cold.sh [rounds=3] [extra -dpcvars, comma list] [label=cold]
set -u
wt="$(cd "$(dirname "$0")/../.." && pwd)"; cd "$wt"
src="/c/Users/kazda/kiro/thirdjam/Game/Packaged/Accept_H28/Windows"
rounds="${1:-3}"; extra="${2:-}"; label="${3:-cold}"
dp="r.ShaderPipelineCache.SaveUserCache=1${extra:+,$extra}"
for k in $(seq 1 "$rounds"); do
  dst="Game/Packaged/A175-$label-$k"
  rm -rf "$dst"; mkdir -p "$dst"; cp -r "$src/." "$dst/"; rm -rf "$dst/ThirdJam/Saved"
  exe="$dst/ThirdJam/Binaries/Win64/ThirdJam.exe"
  for n in 1 2; do
    MSYS_NO_PATHCONV=1 powershell -NoProfile -ExecutionPolicy Bypass -File tools/perf/bot.ps1 -Exe "$exe" -Tier p3_clay_5v5 -Name "a175-$label-$k-$n" \
      -Port 4775 -Offscreen -StartPath play -NoCut -PlaySeconds 0 -Dp "$dp" | tail -1
  done
  ls -la "$dst/ThirdJam/Saved/"*.upipelinecache 2>/dev/null
done
