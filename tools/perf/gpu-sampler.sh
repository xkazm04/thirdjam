#!/usr/bin/env bash
# Lane A58: every 5 s, the GPU utilisation and every game / editor window open on the machine (to label GPU figures busy or quiet).
OUT="$1"; N="${2:-120}"
for i in $(seq 1 $N); do
  u=$(nvidia-smi --query-gpu=utilization.gpu,memory.used --format=csv,noheader 2>/dev/null)
  w=$(powershell -NoProfile -Command "(Get-Process | ? { \$_.MainWindowHandle -ne 0 -and \$_.ProcessName -match 'ThirdJam|Unreal' } | % { \$_.ProcessName + ':' + \$_.Id }) -join ' '")
  echo "$(date +%H:%M:%S) gpu=$u windows=$w" >> "$OUT"; sleep 5
done
