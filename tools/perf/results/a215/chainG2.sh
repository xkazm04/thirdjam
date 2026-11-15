#!/bin/sh
# lane A215: CROSS final (b5): the full Jam filter cleared + the six cells before (neutral seams) / after at 100 a cell, 1 worker (two heavy processes); test-guards; then Shipping
cd "$(dirname "$0")/../../../.."
echo "$(date +%H:%M:%S) chainG2 sh $$" >> Game/logs/a215/pids.txt
( cd tools/perf/results/a215 && W=1 EXTRA="--seam cross_live=0;cross_back=0" sh cells.sh C0 100 && W=1 sh cells.sh C5 100 ) > Game/logs/a215/cells-g2.out 2>&1 &
for v in $(env | grep -o "^JAM_[A-Z0-9_]*"); do unset $v; done; bash tools/perf/results/a215/ue.sh "Jam" gate2
grep -E "Result=\{Fail" Game/logs/a215/gate2.log | sed 's/.*Path={\([^}]*\)}.*/\1/' > Game/logs/a215/gate2-fails.txt
node tools/gate/check-test-guards.mjs > Game/logs/a215/guards2.txt 2>&1; echo "guards exit $?" >> Game/logs/a215/guards2.txt
wait
echo "chainG2 tests+cells done $(date +%H:%M:%S)"
powershell -NoProfile -ExecutionPolicy Bypass -File tools/run/build.ps1 -Project "$(pwd -W)/Game" -Config Shipping -Target Game -TimeoutSec 3000 > Game/logs/a215/ship2.log 2>&1
echo "shipping exit $?"
echo "chainG2 done $(date +%H:%M:%S)"
