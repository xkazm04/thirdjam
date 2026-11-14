#!/bin/sh
# lane A206, build 9a (READY 1's tree): the gate (clean env, Jam), then law 3 (200 seeds a number) as shipped and with brain9=0 at pro and all_star
cd "$(dirname "$0")"
echo "chain1 $$" >> out/pids.txt
./jam.sh gate9a Jam
JAM_LAW3_AT="8,8.5,9,9.5" ./law3.sh law9a "pro:-|pro:brain9=0|all_star:-|all_star:brain9=0"
echo chain1-done >> out/pids.txt
