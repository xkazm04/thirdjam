#!/bin/sh
# lane A201 evolution 12: the ring's candidate rules, obey every ring against ignoring, $2 seeds, level $1
cd "$(dirname "$0")"
echo "r2-$1 $$" >> out/pids.txt
A="ignore@@@@"
for g in "hsec=0.7" "others=0.3" "hsec=0.7;others=0.3" "hsec=0.55"; do A="$A|r$g@@$g@@brake"; done
JAM_TRACE_ARMS="$A" sh trace.sh r2-$1 "level=$1;player=regular;seeds=$2"
echo r2-$1-done
