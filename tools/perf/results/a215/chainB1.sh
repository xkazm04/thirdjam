#!/bin/sh
# lane A215: on b1 (the CROSS rule, game.contact.cross_live_frames ships 10): the neutral proof against B0, the person hand shipped / neutral / without the
# pickup (the eye's question: why the pickup took the cross-check drill from 22 of 30 to 10 of 20), and the tests
cd "$(dirname "$0")"
R=../../../..
mkdir -p out
echo "$(date +%H:%M:%S) chainB1 sh $$" >> out/pids.txt
( cd $R && bash tools/perf/results/a215/ue.sh "Jam.Core.Contact" contact-b1 ) > out/contact-b1.txt &
EXTRA="--seam cross_live=0" sh hashes.sh B1N
for c in rk pro as; do for r in reg large; do node same.mjs B0-$c-$r B1N-$c-$r; done; done > out/B1-neutral.txt
for lv in rk pro; do case $lv in rk) c=novice; l=rookie;; pro) c=regular; l=pro;; esac
  node curve.mjs B1-person-$lv $c $l regular 100 "" --workers 2 --check closing --hand person > /dev/null
  node curve.mjs B1-nopick-$lv $c $l regular 100 "" --workers 2 --check closing --hand person --seam "cross_live=0;pickup_off=1" > /dev/null
done
node hand-table.mjs B0-person-rk B0-person-pro B1-person-rk B1-person-pro B1-nopick-rk B1-nopick-pro > out/B1-person.json
wait
echo "chainB1 done $(date +%H:%M:%S)"
