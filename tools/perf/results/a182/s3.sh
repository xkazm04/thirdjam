#!/bin/sh
# lane A182: 2,000 on the chosen numbers with everything on (regular first), then item 3: the novice season takes 3-7 (200) and three modes (100) with the set on
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a182/chain.pid
R="$(pwd -W)/out/reg-set.cartridge.json"
T="mate.hk_screen_frames=30;mate.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_out=0.7"
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
node curve.mjs F-pro-reg-h14 regular pro regular 2000 "$T;$K;opp.hunt_radius_m=14" --cart "$R" --workers 3 --check closing > /dev/null
node curve.mjs F-rk-reg-red103 novice rookie regular 2000 "$T;red_skill=1.03" --cart "$R" --workers 3 --check closing > /dev/null
for k in 3 4 5 6 7; do
  V="$(pwd -W)/out/take$k-set.cartridge.json"
  if [ $k -le 4 ]; then TT="$T;red_skill=1.03"; else TT="$T;$K;opp.hunt_radius_m=14"; fi
  node curve.mjs S-novice-take-$k novice cart ladder/take-$k 200 "$TT" --cart "$V" --workers 3 --check closing > /dev/null
done
for m in fog-of-war bruisers glass-house; do
  node curve.mjs M-$m regular cart modes/hockey.mode.$m 100 "$T" --cart "$(pwd -W)/out/$m-set.cartridge.json" --workers 3 --check closing > /dev/null
done
echo s3-done
