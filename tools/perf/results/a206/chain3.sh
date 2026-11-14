#!/bin/sh
# lane A206, build 9b (READY 1's tree: rookie without the late read): the gate (clean env), then rookie law 3 as shipped and brain9=0
cd "$(dirname "$0")"
echo "chain3 $$" >> out/pids.txt
./jam.sh gate9b Jam
JAM_LAW3_AT="8,8.5,9" ./law3.sh law9b "rookie:-|rookie:brain9=0"
echo chain3-done >> out/pids.txt
