#!/bin/sh
# lane A215: on b0 (the tree before the CROSS rule): the default match's hashes (B0, 100 a cell, six cells) and the person hand's table at rookie and pro
cd "$(dirname "$0")"
mkdir -p out
echo "$(date +%H:%M:%S) chainB0 sh $$" >> out/pids.txt
sh hashes.sh B0
node curve.mjs B0-person-rk novice rookie regular 100 "" --workers 2 --check closing --hand person > /dev/null
node curve.mjs B0-person-pro regular pro regular 100 "" --workers 2 --check closing --hand person > /dev/null
node hand-table.mjs B0-person-rk B0-person-pro > out/B0-person.json
echo "chainB0 done $(date +%H:%M:%S)"
