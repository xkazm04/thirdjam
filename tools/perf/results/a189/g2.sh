#!/bin/sh
# lane A189 pass 2 (600, closing, brain8=1): pro made easier with drill-neutral carrier numbers: hk_pressure_s 0.3 -> 0.6 / 1.0, better_by 12 -> 6; both rinks
cd "$(dirname "$0")"
echo "g2 $$" > ../../../../Game/logs/a189/chain.pid
node curve.mjs V-pro-reg-hkp06 regular pro regular 600 "brain8=1;opp.hk_pressure_s=0.6" --workers 3 --check closing > /dev/null
node curve.mjs V-pro-large-hkp06 regular pro large 600 "brain8=1;opp.hk_pressure_s=0.6" --workers 3 --check closing > /dev/null
node curve.mjs V-pro-reg-hkp10 regular pro regular 600 "brain8=1;opp.hk_pressure_s=1.0" --workers 3 --check closing > /dev/null
node curve.mjs V-pro-reg-bb6 regular pro regular 600 "brain8=1;opp.hk_better_by_m=6" --workers 3 --check closing > /dev/null
node curve.mjs V-pro-large-hkp10 regular pro large 600 "brain8=1;opp.hk_pressure_s=1.0" --workers 3 --check closing > /dev/null
echo g2-done
