#!/bin/sh
# lane A215: on b2 (the live window at the stick's reach): the person hand shipped, the neutral proof against B0, the contact tests
cd "$(dirname "$0")"
R=../../../..
echo "$(date +%H:%M:%S) chainB2 sh $$" >> out/pids.txt
node curve.mjs B2-person-rk novice rookie regular 100 "" --workers 2 --check closing --hand person > /dev/null
node curve.mjs B2-person-pro regular pro regular 100 "" --workers 2 --check closing --hand person > /dev/null
node hand-table.mjs B2-person-rk B2-person-pro > out/B2-person.json
EXTRA="--seam cross_live=0" sh hashes.sh B2N
for c in rk pro as; do for r in reg large; do node same.mjs B0-$c-$r B2N-$c-$r; done; done > out/B2-neutral.txt
echo "chainB2 done $(date +%H:%M:%S)"
