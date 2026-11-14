#!/bin/sh
# lane A201: the second man's gate without the own-half gate (drill 21 stands in his own half), with and without the slot, obey every light, 200 seeds, level $1
cd "$(dirname "$0")"
echo "m5-$1 $$" >> out/pids.txt
A="ignore@@@"
for g in "second=0.6;slot=12" "second=0.7;slot=12" "second=0.8;slot=12" "second=0.7" "second=0.6;slot=14"; do
  for h in "" 15; do A="$A|s$g h$h@brake@$g@$h"; done
done
JAM_TRACE_ARMS="$(echo "$A" | sed 's/ /_/g')" sh trace.sh m5-$1 "level=$1;player=regular;seeds=200"
echo m5-$1-done
