#!/bin/sh
# lane A201, the proof on the final build: ignore / obey the shipped line (empty rule = FStopRule's defaults) with the class's hand and a quarter second /
# the Director's second arm (second=0.7) / lane A194's line (every gate off), $2 seeds, level $1
cd "$(dirname "$0")"
echo "final-$1 $$" >> out/pids.txt
A="ignore@@@|ship_h@brake@@|ship_h15@brake@@15|strict_h@brake@second=0.7@|strict_h15@brake@second=0.7@15|old_h@brake@second=0;slot=0;own=0@"
JAM_TRACE_ARMS="$A" sh trace.sh final-$1 "level=$1;player=regular;seeds=$2"
echo final-$1-done
