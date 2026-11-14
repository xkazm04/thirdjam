#!/bin/sh
# lane A154: take-5 for the novice (200): pro red 0.35 / 0.37 (as shipped 0.4: 58.5 on this main)
cd "$(dirname "$0")"
cell() { f=$(node take-variant.mjs $2 $3 $4) && node curve.mjs "$1" $5 cart "$f" 200 --workers 3 > /dev/null; }
cell T5-novice-t5-r0.35 5 pro 0.35 novice &
cell T5-novice-t5-r0.37 5 pro 0.37 novice &
wait
