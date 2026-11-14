#!/bin/sh
# lane A162: the season (item 4), the ladder's instrument: the novice takes 1-8 and the regular player's finale (take 8), 200 a take, three workers
cd "$(dirname "$0")"
for t in 1 2 3 4 5 6 7 8; do node curve.mjs S-novice-take-$t novice cart ladder/take-$t 200 --workers 3 --check closing > /dev/null; done
node curve.mjs S-regular-take-8 regular cart ladder/take-8 200 --workers 3 --check closing > /dev/null
node curve.mjs S-good-take-8 good cart ladder/take-8 200 --workers 3 --check closing > /dev/null
