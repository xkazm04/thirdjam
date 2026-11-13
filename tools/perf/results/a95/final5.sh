#!/bin/sh
# lane A95 job 4: the large rink's second step at 2,000 (rookie 93.0 at red 0.86, pro 41.5 at red 1.06)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run F5-rk-large novice rookie large 2000 "red_skill=0.89"
run F5-pro-large regular pro large 2000 "red_skill=1.04"
run F5-as-large good all_star large 2000 "speed_scale=1.22"
run F6-as-large good all_star large 2000 "speed_scale=1.22;opp.hk_pressure_s=0.3"
run F7-rk-large novice rookie large 2000 "red_skill=0.92"
