#!/bin/sh
# lane A201: the own-half rule by time (build 5), obey every light against ignoring, $2 seeds, level $1
cd "$(dirname "$0")"
echo "m6-$1 $$" >> out/pids.txt
A="ignore@@@"
for g in "second=0.7;slot=12;own=1.5" "second=0.65;slot=12;own=1.5" "second=0.7;slot=12;own=1.2" "second=0.7;slot=12;own=99" "second=0.5;slot=12;own=1.5"; do
  for h in "" 15; do A="$A|s$g h$h@brake@$g@$h"; done
done
JAM_TRACE_ARMS="$(echo "$A" | sed 's/ /_/g')" sh trace.sh m6-$1 "level=$1;player=regular;seeds=$2"
echo m6-$1-done
