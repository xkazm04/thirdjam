#!/bin/sh
# lane A215: CROSS's gate on b1: law 3 (shipped), the full Jam filter in a cleared environment, test-guards; then the Shipping compile of the Game target
cd "$(dirname "$0")/../../../.."
echo "$(date +%H:%M:%S) chainG1 sh $$" >> Game/logs/a215/pids.txt
( EXTRA_ENV="JAM_LAW3_AT=8,8.5,9,9.5" sh tools/perf/results/a215/law3.sh G1-law3 "pro:-|all_star:-" ) &
for v in $(env | grep -o "^JAM_[A-Z0-9_]*"); do unset $v; done; bash tools/perf/results/a215/ue.sh "Jam" gate1
grep -E "Result=\{Fail" Game/logs/a215/gate1.log | sed 's/.*Path={\([^}]*\)}.*/\1/' > Game/logs/a215/gate1-fails.txt
wait
node tools/gate/check-test-guards.mjs > Game/logs/a215/guards1.txt 2>&1; echo "guards exit $?" >> Game/logs/a215/guards1.txt
echo "chainG1 done $(date +%H:%M:%S)"
