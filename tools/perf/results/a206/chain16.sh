#!/bin/sh
# lane A206, build m8 (READY 3): gate, law 3, the neutral hash runs, the bars (60 a level)
cd "$(dirname "$0")"
echo "chain16 $$" >> out/pids.txt
./jam.sh gatem8 Jam
JAM_LAW3_AT="8,8.5,9,9.5" ./law3.sh lawm8 "rookie:-|pro:-|all_star:-"
../../../../tools/perf/results/a206/interest.sh n8rk0 "levels=rookie;seeds=100;player=regular;tune=brain9=0"
../../../../tools/perf/results/a206/interest.sh n8pro0 "levels=pro;seeds=100;player=regular;tune=brain9=0"
../../../../tools/perf/results/a206/interest.sh q8 "levels=rookie,pro;seeds=60;player=regular"
echo chain16-done >> out/pids.txt
