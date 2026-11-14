#!/bin/sh
# lane A182: everything on at 2,000: all_star regular (the gap) and rookie large (as shipped, 1.14)
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a182/chain.pid
R="$(pwd -W)/out/reg-set.cartridge.json"; L="$(pwd -W)/out/large-set.cartridge.json"
T="mate.hk_screen_frames=30;mate.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_out=0.7"
for c in "N-rk-reg novice rookie" "N-pro-reg regular pro" "N-as-reg good all_star"; do set -- $c; node curve.mjs $1 $2 $3 regular 600 --workers 3 --check closing > /dev/null; done
node curve.mjs F-as-reg-set good all_star regular 2000 "$T" --cart "$R" --workers 3 --check closing > /dev/null
node curve.mjs F-rk-large-set novice rookie large 2000 "$T" --cart "$L" --workers 3 --check closing > /dev/null
echo s5-done
