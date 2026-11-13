#!/bin/sh
# Lane A86: the large target cells left from the baseline, then one lever at a time at n 600 (regular rink). brain reach = the shipped rule from now.
cd "$(dirname "$0")/../../../.."
D=tools/perf/results/a86/curve.mjs
B="--brain reach"
node $D bL-regular-pro regular pro large 2000 "" $B
node $D bL-good-allstar good all_star large 2000 "" $B
node $D l-r-red0.9 novice rookie regular 600 "red_skill=0.9" $B
node $D l-r-red0.95 novice rookie regular 600 "red_skill=0.95" $B
node $D l-r-chk6 novice rookie regular 600 "opp.check_min_closing=6" $B
node $D l-r-red0.9-chk6 novice rookie regular 600 "red_skill=0.9;opp.check_min_closing=6" $B
node $D l-p-red0.9 regular pro regular 600 "red_skill=0.9" $B
node $D l-p-red0.85 regular pro regular 600 "red_skill=0.85" $B
node $D l-p-spd0.95 regular pro regular 600 "speed_scale=0.95" $B
node $D l-p-spd0.9 regular pro regular 600 "speed_scale=0.9" $B
node $D l-a-spd1.1 good all_star regular 600 "speed_scale=1.1" $B
node $D l-a-spd1.0 good all_star regular 600 "speed_scale=1.0" $B
node $D l-a-spd1.1-chk2 good all_star regular 600 "speed_scale=1.1;opp.check_min_closing=2" $B
node $D l-a-spd1.05 good all_star regular 600 "speed_scale=1.05" $B
