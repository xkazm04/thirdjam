#!/usr/bin/env bash
# Lane A132: the arms of build 2, one batch of three processes at a time, memory checked before each batch (over 50 GB in use: wait).
# Usage: bash run-arms.sh <arm> <n> <set|-> [ENV=value ...]   (one arm a call; chain calls with ;)
here="$(cd "$(dirname "$0")" && pwd)"
used() { powershell -NoProfile -Command "\$o=Get-CimInstance Win32_OperatingSystem; [math]::Round((\$o.TotalVisibleMemorySize-\$o.FreePhysicalMemory)/1MB,1)" | tr -d '\r'; }
u=$(used); n=0
while awk "BEGIN{exit !($u > 50)}"; do
  [ $n -eq 0 ] && echo "memory $u GB in use: waiting for under 50"
  sleep 10; n=$((n+1)); u=$(used)
  [ $n -gt 60 ] && { echo "memory still $u GB after 10 minutes: not started"; exit 2; }
done
echo "memory $u GB in use: starting arm $1 ($(date +%H:%M:%S))"
node "$here/cells.mjs" "$@"
