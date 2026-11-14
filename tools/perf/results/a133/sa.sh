#!/bin/sh
# lane A133 evolution 3, round A (200 a cell, three processes): takes 4-7 at pro with red 0.7 / 0.8 / 0.9 for the novice; take 6 as is; take 8 at all_star red 0.7 / 0.8
cd "$(dirname "$0")"
cell() { f=$(node take-variant.mjs $2 $3 $4) && node curve.mjs "$1" $5 cart "$f" 200 --workers 3 > /dev/null; }
node curve.mjs SA-novice-t6 novice cart ladder/take-6 200 --workers 3 > /dev/null
for t in 4 5 6 7; do for r in 0.7 0.8 0.9; do cell SA-novice-t$t-pro-r$r $t pro $r novice; done; done
for r in 0.7 0.8; do for k in regular good; do cell SA-$k-t8-as-r$r 8 all_star $r $k; done; done
