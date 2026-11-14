#!/bin/sh
# lane A170 item 1: 2,000 at hunt 11 and 12 (regular), then the large rink's pro cell at both; closing press, three workers, one chain
cd "$(dirname "$0")"
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
node curve.mjs F-pro-reg-h11 regular pro regular 2000 "$K;opp.hunt_radius_m=11" --workers 3 --check closing > /dev/null
node curve.mjs F-pro-reg-h12 regular pro regular 2000 "$K;opp.hunt_radius_m=12" --workers 3 --check closing > /dev/null
node curve.mjs F-pro-large-h11 regular pro large 2000 "$K;opp.hunt_radius_m=11" --workers 3 --check closing > /dev/null
node curve.mjs F-pro-large-h12 regular pro large 2000 "$K;opp.hunt_radius_m=12" --workers 3 --check closing > /dev/null
