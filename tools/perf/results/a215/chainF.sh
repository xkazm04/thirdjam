#!/bin/sh
# lane A215: CROSS final on b6: the person hand shipped and neutral (t_c_back: the bots' cross-checks on his back), the neutral hashes, then chainG2 (gate, cells, guards, Shipping)
cd "$(dirname "$0")"
echo "$(date +%H:%M:%S) chainF sh $$" >> out/pids.txt
for lv in rk pro; do case $lv in rk) c=novice; l=rookie;; pro) c=regular; l=pro;; esac
  node curve.mjs F-person-$lv $c $l regular 100 "" --workers 2 --check closing --hand person > /dev/null
  node curve.mjs FN-person-$lv $c $l regular 100 "" --workers 2 --check closing --hand person --seam "cross_live=0;cross_back=0" > /dev/null
done
node hand-table.mjs B0-person-rk B0-person-pro FN-person-rk FN-person-pro F-person-rk F-person-pro > out/F-person.json
EXTRA="--seam cross_live=0;cross_back=0" sh hashes.sh FN
for c in rk pro as; do for r in reg large; do node same.mjs B0-$c-$r FN-$c-$r; done; done > out/F-neutral.txt
echo "chainF person+neutral done $(date +%H:%M:%S)"
sh chainG2.sh
echo "chainF done $(date +%H:%M:%S)"
