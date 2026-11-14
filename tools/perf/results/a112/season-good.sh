#!/bin/sh
# lane A112: the season in order, the good playtester, each take at its own level, 200 seeds a take (whistle off, take-3 rookie)
cd "$(dirname "$0")"
for t in 1 2 3 4 5 6 7 8; do node curve.mjs S-good-take-$t good cart ladder/take-$t 200 --workers 2 > /dev/null; done
