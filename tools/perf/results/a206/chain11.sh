#!/bin/sh
# lane A206, build m5 (READY 2 candidate): gate, law 3 (as shipped and brain9=0 at every level)
cd "$(dirname "$0")"
echo "chain11 $$" >> out/pids.txt
./jam.sh gatem5 Jam
JAM_LAW3_AT="8,8.5,9,9.5" ./law3.sh lawm5 "rookie:-|rookie:brain9=0|pro:-|pro:brain9=0|all_star:-|all_star:brain9=0"
echo chain11-done >> out/pids.txt
