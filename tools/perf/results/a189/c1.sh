#!/bin/sh
# lane A189 pass 1, build 2 (the numbers compiled in): the six cells as built at 2,000 (two workers) beside the full Jam filter in a clean environment (one process);
# built = tune by seed on the shared seeds
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a189/chain.pid
sh jam.sh jam-1 > ../../../../Game/logs/a189/jam-1.sum &
echo "jam pid $!" >> ../../../../Game/logs/a189/chain.pid
node curve.mjs L-rk-reg novice rookie regular 2000 --workers 2 --check closing > /dev/null; node same.mjs L-rk-reg R-rk-reg-red080-shoot20
node curve.mjs L-as-large good all_star large 2000 --workers 2 --check closing > /dev/null; node same.mjs L-as-large A-as-large-sp1.62
node curve.mjs L-as-reg good all_star regular 2000 --workers 2 --check closing > /dev/null; node same.mjs L-as-reg A-as-reg-sp1.5
node curve.mjs L-pro-reg regular pro regular 2000 --workers 2 --check closing > /dev/null; node same.mjs L-pro-reg P-pro-reg-h11
node curve.mjs L-pro-large regular pro large 2000 --workers 2 --check closing > /dev/null; node same.mjs L-pro-large P-pro-large-h11
node curve.mjs L-rk-large novice rookie large 2000 --workers 2 --check closing > /dev/null
wait
cat ../../../../Game/logs/a189/jam-1.sum
echo c1-done
