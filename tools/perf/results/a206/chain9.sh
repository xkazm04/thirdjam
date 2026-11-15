#!/bin/sh
# lane A206, build w2a (wave 2 candidate): gate, law 3 all levels, then the six cells at 600
cd "$(dirname "$0")"
echo "chain9 $$" >> out/pids.txt
./jam.sh gatew2a Jam
JAM_LAW3_AT="8,8.5,9,9.5" ./law3.sh laww2a "rookie:-|pro:-|all_star:-"
echo chain9-done >> out/pids.txt
