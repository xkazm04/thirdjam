#!/bin/sh
# lane A201: the finer sweep between G1 and G2 (the second man 0.55 / 0.6 / 0.65 s; the own end 35 m), obey every light, 200 seeds, level $1
cd "$(dirname "$0")"
echo "m4-$1 $$" >> out/pids.txt
A="ignore@@@"
for g in "second=0.55;slot=12;farnet=40" "second=0.6;slot=12;farnet=40" "second=0.65;slot=12;farnet=40" "second=0.6;slot=12;farnet=35" "second=0.6;farnet=40"; do
  for h in "" 15; do A="$A|s$g h$h@brake@$g@$h"; done
done
JAM_TRACE_ARMS="$(echo "$A" | sed 's/ /_/g')" sh trace.sh m4-$1 "level=$1;player=regular;seeds=200"
echo m4-$1-done
