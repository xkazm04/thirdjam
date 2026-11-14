#!/bin/sh
# lane A201: obey every lit line against ignoring it, by the line's gates and the obeying hand's lag (6 = the regular class's own; 15 = the quarter second the line is lit for), 200 seeds, level $1
cd "$(dirname "$0")"
echo "m3-$1 $$" >> out/pids.txt
G1="second=0.5;slot=12;farnet=40"; G2="second=0.7;slot=12;farnet=40"; G3="second=0.4;farnet=40"; G4="farnet=40;slot=12"
A="ignore@@@|obey6@brake@@|obey15@brake@@15|g1h6@brake@$G1@|g1h15@brake@$G1@15|g2h6@brake@$G2@|g2h15@brake@$G2@15|g3h15@brake@$G3@15|g4h15@brake@$G4@15"
JAM_TRACE_ARMS="$A" sh trace.sh m3-$1 "level=$1;player=regular;seeds=200"
echo m3-$1-done
