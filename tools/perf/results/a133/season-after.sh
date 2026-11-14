#!/bin/sh
# lane A133 evolution 3: the season as written (build 6), 200 a take and class, two workers
cd "$(dirname "$0")"
for k in novice regular good; do for t in 1 2 3 4 5 6 7 8; do node curve.mjs SF-$k-take-$t $k cart ladder/take-$t 200 --workers 2 > /dev/null; done; done
