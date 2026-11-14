#!/bin/sh
# lane A189 pass 2 (600, closing, brain8=1, pressure 0.6): pro large red 1.165 / 1.15
cd "$(dirname "$0")"
echo "g3 $$" > ../../../../Game/logs/a189/chain.pid
for r in 1.165 1.15; do node curve.mjs V-pro-large-hkp06-red$r regular pro large 600 "brain8=1;opp.hk_pressure_s=0.6;red_skill=$r" --workers 3 --check closing > /dev/null; done
echo g3-done
