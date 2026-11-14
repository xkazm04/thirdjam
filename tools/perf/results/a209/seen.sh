#!/bin/sh
# lane A209 item 1: the person-like hand and the scripted one at rookie and pro, 50 matches each, regular rink, the audit in every line.
cd "$(dirname "$0")"
mkdir -p out
echo "$(date +%H:%M:%S) chain-seen$1 sh $$" >> out/pids.txt
P=${1:-S0}
node curve.mjs $P-person-rk novice rookie regular 50 "" --workers 2 --check closing --hand person > /dev/null
node curve.mjs $P-person-pro regular pro regular 50 "" --workers 2 --check closing --hand person > /dev/null
node curve.mjs $P-script-rk novice rookie regular 50 "" --workers 2 --check closing > /dev/null
node curve.mjs $P-script-pro regular pro regular 50 "" --workers 2 --check closing > /dev/null
node hand-table.mjs $P-person-rk $P-person-pro $P-script-rk $P-script-pro
echo "chain-seen$P done $(date +%H:%M:%S)"
