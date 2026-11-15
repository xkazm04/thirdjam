#!/bin/sh
# lane A209 wave 3: the person hand on m2 at the three levels and a dodging arm at pro (the hits taken)
cd "$(dirname "$0")"
echo "$(date +%H:%M:%S) chain3 sh $$" >> out/pids.txt
node curve.mjs W14-rk novice rookie regular 100 "" --workers 2 --check closing --hand person > /dev/null
node curve.mjs W14-pro regular pro regular 100 "" --workers 2 --check closing --hand person > /dev/null
node curve.mjs W14-as good all_star regular 100 "" --workers 2 --check closing --hand person > /dev/null
JAM_PLAYTESTER_DODGE=on node curve.mjs W14-pro-dodge regular pro regular 100 "" --workers 2 --check closing --hand person > /dev/null
node hand-table.mjs W14-rk W14-pro W14-as W14-pro-dodge > out/W14.json
echo "chain3 done $(date +%H:%M:%S)"
