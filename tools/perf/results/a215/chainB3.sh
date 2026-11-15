#!/bin/sh
# lane A215: on b3 (CROSS: the live window at the stick's reach + the human's tap in a carrier's back): tests, the person hand, the neutral proof, the six cells before / after
cd "$(dirname "$0")"
R=../../../..
echo "$(date +%H:%M:%S) chainB3 sh $$" >> out/pids.txt
( cd $R && bash tools/perf/results/a215/ue.sh "Jam.Core.Contact" contact-b3 ) > out/contact-b3.txt
node curve.mjs B3-person-rk novice rookie regular 100 "" --workers 2 --check closing --hand person > /dev/null
node curve.mjs B3-person-pro regular pro regular 100 "" --workers 2 --check closing --hand person > /dev/null
node hand-table.mjs B0-person-rk B0-person-pro B3-person-rk B3-person-pro > out/B3-person.json
EXTRA="--seam cross_live=0;cross_back=0" sh hashes.sh B3N
for c in rk pro as; do for r in reg large; do node same.mjs B0-$c-$r B3N-$c-$r; done; done > out/B3-neutral.txt
echo "chainB3 person+neutral done $(date +%H:%M:%S)"
EXTRA="--seam cross_live=0;cross_back=0" sh cells.sh C0 200
sh cells.sh C3 200
echo "chainB3 done $(date +%H:%M:%S)"
