#!/bin/sh
# lane A95 job 4, round 7: rookie without tightening its principles (late and loose stays: share 0.6, 24 frames): red, checks from 6 m/s, sprint; 600 per cell
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run L7-rk-A novice rookie regular 600 "red_skill=0.95;opp.check_min_closing=6;opp.sprint_share=0.2"
run L7-rk-B novice rookie regular 600 "red_skill=1.0;opp.check_min_closing=6;opp.sprint_share=0.2"
