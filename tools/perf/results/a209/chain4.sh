#!/bin/sh
# lane A209 wave 3: m3 (min meet off, the bots' pull as a seam): the person hand shipped, and the bots' pull 0.5 at pro / all_star, law 3 with it
cd "$(dirname "$0")"
echo "$(date +%H:%M:%S) chain4 sh $$" >> out/pids.txt
( EXTRA_ENV="JAM_LAW3_AT=8,8.5,9,9.5 JAM_TEST_A151=bot_pull=0.5" sh law3.sh m3-law3-bp05 "pro:-|all_star:-" ) &
node curve.mjs W15-rk novice rookie regular 100 "" --workers 2 --check closing --hand person > /dev/null
node curve.mjs W15-pro regular pro regular 100 "" --workers 2 --check closing --hand person > /dev/null
node curve.mjs W15-pro-bp regular pro regular 100 "" --workers 2 --check closing --hand person --seam "bot_pull=0.5" > /dev/null
node curve.mjs W15-as-bp good all_star regular 100 "" --workers 2 --check closing --hand person --seam "bot_pull=0.5" > /dev/null
wait
node hand-table.mjs W15-rk W15-pro W15-pro-bp W15-as-bp > out/W15.json
echo "chain4 done $(date +%H:%M:%S)"
