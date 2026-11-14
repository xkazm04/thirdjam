#!/bin/sh
# lane A201 item 1, the paired instrument: ignore, and k1..k12 (obey only the N-th light), 200 seeds, at level $1 (one process)
cd "$(dirname "$0")"
echo "m2-$1 $$" >> out/pids.txt
A="ignore@@"; for k in 1 2 3 4 5 6 7 8 9 10 11 12; do A="$A|k$k@brake:$k@"; done
JAM_TRACE_ARMS="$A" sh trace.sh m2-$1 "level=$1;player=regular;seeds=200"
echo m2-$1-done
