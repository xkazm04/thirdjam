#!/bin/sh
# lane A206, build 9d (READY 1's tree): the gate (clean env), then law 3: rookie, pro, all_star as shipped and brain9=0
cd "$(dirname "$0")"
echo "chain7 $$" >> out/pids.txt
./jam.sh gate9d Jam
JAM_LAW3_AT="8,8.5,9,9.5" ./law3.sh law9d "rookie:-|rookie:brain9=0|pro:-|pro:brain9=0|all_star:-|all_star:brain9=0"
echo chain7-done >> out/pids.txt
