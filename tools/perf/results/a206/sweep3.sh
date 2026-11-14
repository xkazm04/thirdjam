#!/bin/sh
# lane A206: rookie's poke (law 3's straight charge at rookie, and the cell)
cd "$(dirname "$0")"
echo "sweep3 $$" >> out/pids.txt
JAM_LAW3_AT="8.5" ./law3.sh law9p "rookie:opp.hk_poke9_frames=12|rookie:opp.hk_poke9_frames=14|rookie:opp.hk_poke9_frames=16" &
node curve.mjs R3-poke12 novice rookie regular 200 "opp.hk_poke9_frames=12" --workers 1 --check closing > /dev/null &
node curve.mjs R3-poke14 novice rookie regular 200 "opp.hk_poke9_frames=14" --workers 1 --check closing > /dev/null &
wait
echo sweep3-done >> out/pids.txt
