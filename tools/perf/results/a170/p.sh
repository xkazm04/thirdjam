#!/bin/sh
# lane A170: the pinned variant built (build 5): three modes and hockey.chaos as built (pinned at 8) against the shipped tree's rows, 100 seeds by seed; pro regular as built against the tune rows; beside the full Jam filter
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a170/chain.pid
sh jam.sh jam-3 > ../../../../Game/logs/a170/jam-3.sum &
for m in fog-of-war bruisers glass-house; do node curve.mjs P-$m regular cart modes/hockey.mode.$m 100 --workers 2 --check closing > /dev/null; node same.mjs P-$m M-$m; done
node curve.mjs P-chaos regular cart hockey.chaos 100 --workers 2 --check closing > /dev/null; node same.mjs P-chaos M-chaos
node curve.mjs P-pro-reg regular pro regular 600 --workers 2 --check closing > /dev/null; node same.mjs P-pro-reg D-pro-reg-h11
node curve.mjs P-novice-take-5 novice cart ladder/take-5 200 --workers 2 --check closing > /dev/null; node same.mjs P-novice-take-5 SD-novice-take-5
wait
cat ../../../../Game/logs/a170/jam-3.sum
echo p-done
