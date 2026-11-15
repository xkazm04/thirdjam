#!/bin/sh
cd "$(dirname "$0")"
echo "$(date +%H:%M:%S) chain5 sh $$" >> out/pids.txt
( EXTRA_ENV="JAM_LAW3_AT=8,8.5,9,9.5 JAM_TEST_A151=bot_pull=0.3" sh law3.sh m4-law3-bp03 "pro:-|all_star:-" ) &
node curve.mjs W16-pro-bp03 regular pro regular 100 "" --workers 2 --check closing --hand person --seam "bot_pull=0.3" > /dev/null
node curve.mjs W16-as-bp03 good all_star regular 100 "" --workers 2 --check closing --hand person --seam "bot_pull=0.3" > /dev/null
wait
node hand-table.mjs W16-pro-bp03 W16-as-bp03 > out/W16.json
echo "chain5 done $(date +%H:%M:%S)"
