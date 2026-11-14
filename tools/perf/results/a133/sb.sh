#!/bin/sh
# lane A133 evolution 3, round B (200 a cell, three processes): the chain take-4 rookie (81.5) -> take-5 >= 61.5 -> take-6 >= 41.5 -> take-7 >= 21.5
cd "$(dirname "$0")"
cell() { f=$(node take-variant.mjs $2 $3 $4) && node curve.mjs "$1" $5 cart "$f" 200 --workers 3 > /dev/null; }
cell SB-novice-t5-pro-r0.3 5 pro 0.3 novice
cell SB-novice-t5-pro-r0.45 5 pro 0.45 novice
cell SB-novice-t6-pro-r0.5 6 pro 0.5 novice
cell SB-novice-t6-pro-r0.6 6 pro 0.6 novice
cell SB-novice-t7-pro-r0.7 7 pro 0.7 novice
cell SB-novice-t7-pro-r0.8 7 pro 0.8 novice
cell SB-regular-t8-as-r0.7 8 all_star 0.7 regular
cell SB-good-t8-as-r0.7 8 all_star 0.7 good
