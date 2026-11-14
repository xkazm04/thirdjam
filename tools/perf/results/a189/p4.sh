#!/bin/sh
# lane A189 pass 1, pro (600, closing): hunt 10 and 9 regular; hunt 11 large
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a189/chain.pid
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
node curve.mjs P-pro-reg-h10 regular pro regular 600 "$K;opp.hunt_radius_m=10" --workers 3 --check closing > /dev/null
node curve.mjs P-pro-reg-h9 regular pro regular 600 "$K;opp.hunt_radius_m=9" --workers 3 --check closing > /dev/null
node curve.mjs P-pro-large-h11 regular pro large 600 "$K;opp.hunt_radius_m=11" --workers 3 --check closing > /dev/null
echo p4-done
