#!/bin/sh
# lane A133 evolution 3: takes 4-8 as they were before it (pro / all_star, no red_skill), the regular and good players, 200 a take, one worker
cd "$(dirname "$0")"
cell() { f=$(node take-variant.mjs $2 $3) && node curve.mjs "$1" $4 cart "$f" 200 --workers 1 > /dev/null; }
for k in regular good; do for t in 4 5 6 7; do cell SO-$k-take-$t $t pro $k; done; cell SO-$k-take-8 8 all_star $k; done
