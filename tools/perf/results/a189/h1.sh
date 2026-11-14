#!/bin/sh
# lane A189 LADDER SET 2, build 9 (pressure 0.6, pro large red 1.165 compiled in; brain8 by tune): full Jam (env cleared) beside 2,000 of pro regular and large (two workers), built = tune by seed; then law 3 with the set
cd "$(dirname "$0")"
echo "h1 $$" > ../../../../Game/logs/a189/chain.pid
sh jam.sh jam-4 > ../../../../Game/logs/a189/jam-4.sum &
node curve.mjs T-pro-reg regular pro regular 2000 "brain8=1" --workers 2 --check closing > /dev/null; node same.mjs T-pro-reg V-pro-reg-hkp06
node curve.mjs T-pro-large regular pro large 2000 "brain8=1" --workers 2 --check closing > /dev/null; node same.mjs T-pro-large V-pro-large-hkp06-red1.165
wait
env -u JAM_PLAYTESTER_CHECK sh law3.sh law3-4 "pro:brain8=1|pro:-"
sed 's/ | lifted.*//' ../../../../Game/logs/a189/law3-4.txt
cat ../../../../Game/logs/a189/jam-4.sum
echo h1-done
