#!/bin/sh
# lane A133 evolution 3, round C (200 a cell): take-5 pro red 0.4; take-7 pro red 0.9 / 0.95
cd "$(dirname "$0")"
cell() { f=$(node take-variant.mjs $2 $3 $4) && node curve.mjs "$1" $5 cart "$f" 200 --workers 3 > /dev/null; }
cell SC-novice-t5-pro-r0.4 5 pro 0.4 novice
cell SC-novice-t7-pro-r0.9 7 pro 0.9 novice
cell SC-novice-t7-pro-r0.95 7 pro 0.95 novice
