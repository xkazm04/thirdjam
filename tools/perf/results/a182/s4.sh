#!/bin/sh
# lane A182: pro large at 2,000, hunt 14, everything on
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a182/chain.pid
L="$(pwd -W)/out/large-set.cartridge.json"
T="mate.hk_screen_frames=30;mate.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_out=0.7"
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
node curve.mjs F-pro-large-h14 regular pro large 2000 "$T;$K;opp.hunt_radius_m=14" --cart "$L" --workers 3 --check closing > /dev/null
echo s4-done
