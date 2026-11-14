#!/bin/sh
# lane A162: build 2 as built (the two numbers in source), 600 a cell, two workers, beside the full Jam filter (one process)
cd "$(dirname "$0")"
sh jam.sh jam-1 > ../../../../Game/logs/a162/jam-1.sum &
node curve.mjs C-pro-reg regular pro regular 600 --workers 2 --check closing > /dev/null
node curve.mjs C-rk-large novice rookie large 600 --workers 2 --check closing > /dev/null
node same.mjs C-pro-reg H-pro-reg-red113; node same.mjs C-rk-large G-rk-large-red114
wait
cat ../../../../Game/logs/a162/jam-1.sum
