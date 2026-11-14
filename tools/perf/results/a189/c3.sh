#!/bin/sh
# lane A189 LADDER SET 1, build 5 (rookie red 0.80 / reach 20 / large red 1.11; pro large red 1.18): full Jam (env cleared) beside 2,000 as built of the three changed cells (two workers), built = tune by seed
cd "$(dirname "$0")"
echo "c3 $$" > ../../../../Game/logs/a189/chain.pid
sh jam.sh jam-3 > ../../../../Game/logs/a189/jam-3.sum &
node curve.mjs S1-rk-reg novice rookie regular 2000 --workers 2 --check closing > /dev/null; node same.mjs S1-rk-reg N-rk-reg-tune
node curve.mjs S1-pro-large regular pro large 2000 --workers 2 --check closing > /dev/null; node same.mjs S1-pro-large N-pro-large-red1.18
node curve.mjs S1-rk-large novice rookie large 2000 --workers 2 --check closing > /dev/null; node same.mjs S1-rk-large N-rk-large-red1.11
wait
cat ../../../../Game/logs/a189/jam-3.sum
echo c3-done
