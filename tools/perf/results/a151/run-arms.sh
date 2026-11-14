#!/usr/bin/env bash
# Lane A151: arms of cells.mjs in sequence (three processes each), memory checked before each (over 50 GB in use: wait).
#   bash run-arms.sh "<arm> <n> <set|-> [ENV=value ...]" ...
cd "$(dirname "$0")/../../../.."
for spec in "$@"; do
  while :; do
    used=$(powershell -NoProfile -Command '$o=Get-CimInstance Win32_OperatingSystem; [math]::Round(($o.TotalVisibleMemorySize-$o.FreePhysicalMemory)/1MB,1)' | tr -d '\r')
    if awk "BEGIN{exit !($used < 50)}"; then break; fi
    echo "memory $used GB in use: waiting"; sleep 20
  done
  echo "== $spec (memory $used GB) $(date +%H:%M:%S)"
  # shellcheck disable=SC2086
  node tools/perf/results/a151/cells.mjs $spec
done
echo "arms done $(date +%H:%M:%S)"
