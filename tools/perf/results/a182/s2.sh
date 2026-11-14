#!/bin/sh
# lane A182: the large rink with everything on: pro hunt 14 / 16 / 18, rookie red 1.17 (600, closing)
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a182/chain.pid
L="$(pwd -W)/out/large-set.cartridge.json"
T="mate.hk_screen_frames=30;mate.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_out=0.7"
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
for h in 14 16 18; do node curve.mjs X-pro-large-h$h regular pro large 600 "$T;$K;opp.hunt_radius_m=$h" --cart "$L" --workers 3 --check closing > /dev/null; done
node curve.mjs X-rk-large-red117 novice rookie large 600 "$T;red_skill=1.17" --cart "$L" --workers 3 --check closing > /dev/null
echo s2-done
