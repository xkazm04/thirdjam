#!/bin/sh
# lane A95 job 4: the profiles as built (no tune): the large rink's three target cells at 600 first (its offsets), then the final 2,000 on hockey.regular
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run P-rk-large novice rookie large 600
run P-pro-large regular pro large 600
run P-as-large good all_star large 600
run F-rk-regular novice rookie regular 2000
run F-pro-regular regular pro regular 2000
run F-as-regular good all_star regular 2000
