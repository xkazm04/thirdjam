#!/bin/sh
# lane A189 step 3: rookie large with reach 20 and large red 1.11 / 1.09 (600, closing)
cd "$(dirname "$0")"
echo "d3 $$" > ../../../../Game/logs/a189/chain.pid
for r in 1.11 1.09; do node curve.mjs N-rk-large-red$r novice rookie large 600 "opp.shoot_extra_m=20;red_skill=$r" --workers 3 --check closing > /dev/null; done
echo d3-done
