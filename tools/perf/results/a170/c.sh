#!/bin/sh
# lane A170: build 2 (the patch compiled in) as built, 600 a cell, two workers, against the tune rows by seed, beside the full Jam filter (one process)
cd "$(dirname "$0")"
sh jam.sh jam-1 > ../../../../Game/logs/a170/jam-1.sum &
node curve.mjs C-pro-reg regular pro regular 600 --workers 2 --check closing > /dev/null
node curve.mjs C-pro-large regular pro large 600 --workers 2 --check closing > /dev/null
node same.mjs C-pro-reg D-pro-reg-h11; node same.mjs C-pro-reg F-pro-reg-h11; node same.mjs C-pro-large F-pro-large-h11
wait
cat ../../../../Game/logs/a170/jam-1.sum
echo c-done
