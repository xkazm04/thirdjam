#!/bin/sh
# lane A154: the season on main 4cab387 as shipped, the novice and the regular player, 200 a take, three workers
cd "$(dirname "$0")"
for k in novice regular; do for t in 1 2 3 4 5 6 7 8; do node curve.mjs S-$k-take-$t $k cart ladder/take-$t 200 --workers 3 > /dev/null; done; done
