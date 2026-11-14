#!/bin/sh
# lane A201 evolution 12, the proof on the final build: ignore the ring / obey every ring as shipped / obey today's ring (hsec=1;others=0), $2 seeds, level $1
cd "$(dirname "$0")"
echo "final2-$1 $$" >> out/pids.txt
JAM_TRACE_ARMS="ignore@@@@|ring_ship@@@@brake|ring_old@@hsec=1;others=0@@brake" sh trace.sh final2-$1 "level=$1;player=regular;seeds=$2"
echo final2-$1-done
