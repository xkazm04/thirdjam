#!/bin/sh
# lane A206, build m6 (READY 2): gate, law 3 (as shipped, every level), the neutral hash runs (brain9=0 rookie and pro x 100 against m1's)
cd "$(dirname "$0")"
echo "chain13 $$" >> out/pids.txt
./jam.sh gatem6 Jam
JAM_LAW3_AT="8,8.5,9,9.5" ./law3.sh lawm6 "rookie:-|pro:-|all_star:-"
../../../../tools/perf/results/a206/interest.sh n6rk0 "levels=rookie;seeds=100;player=regular;tune=brain9=0"
../../../../tools/perf/results/a206/interest.sh n6pro0 "levels=pro;seeds=100;player=regular;tune=brain9=0"
echo chain13-done >> out/pids.txt
