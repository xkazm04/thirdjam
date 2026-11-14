#!/bin/sh
# lane A189 pass 1, build 3 (hunt 10, the test fix): full Jam (env cleared) beside pro regular and large at 2,000 as built (two workers); built = tune by seed; then rookie large
cd "$(dirname "$0")"
echo "c2 $$" > ../../../../Game/logs/a189/chain.pid
sh jam.sh jam-2 > ../../../../Game/logs/a189/jam-2.sum &
echo "jam $!" >> ../../../../Game/logs/a189/chain.pid
node curve.mjs L2-pro-reg regular pro regular 2000 --workers 2 --check closing > /dev/null; node same.mjs L2-pro-reg P-pro-reg-h10
node curve.mjs L2-pro-large-red117 regular pro large 600 "red_skill=1.17" --workers 2 --check closing > /dev/null
node curve.mjs L2-pro-large-red119 regular pro large 600 "red_skill=1.19" --workers 2 --check closing > /dev/null
node curve.mjs L2-rk-large novice rookie large 600 --workers 2 --check closing > /dev/null
wait
cat ../../../../Game/logs/a189/jam-2.sum
echo c2-done
