#!/bin/sh
# lane A206, build 9c (READY 1's tree: rookie eased by execution): the gate (clean env), then rookie law 3 as shipped and brain9=0
cd "$(dirname "$0")"
echo "chain5 $$" >> out/pids.txt
./jam.sh gate9c Jam
JAM_LAW3_AT="8,8.5,9" ./law3.sh law9c "rookie:-|rookie:brain9=0"
echo chain5-done >> out/pids.txt
