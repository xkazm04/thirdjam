#!/bin/sh
# lane A201 evolution 12: the ring as shipped: ignore it, obey every ring, and the paired arms k1..k10 (obey only the N-th ring), $2 seeds, level $1
cd "$(dirname "$0")"
echo "r1-$1 $$" >> out/pids.txt
A="ignore@@@@|obey@@@@brake"; for k in 1 2 3 4 5 6 7 8 9 10; do A="$A|k$k@@@@brake:$k"; done
JAM_TRACE_ARMS="$A" sh trace.sh r1-$1 "level=$1;player=regular;seeds=$2"
echo r1-$1-done
