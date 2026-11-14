#!/bin/sh
# lane A112: the profile as built (no tune), whistle off in the regular cuts (content applied), 600 a cell, both rinks
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
run W-rk-reg novice rookie regular 600
run W-pro-reg regular pro regular 600
run W-as-reg good all_star regular 600
run W-rk-large novice rookie large 600
run W-pro-large regular pro large 600
run W-as-large good all_star large 600
