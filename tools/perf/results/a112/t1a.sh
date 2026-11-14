#!/bin/sh
# lane A112 trim on the whistle-off tree (as built + tune), 600
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 > /dev/null; }
run T1-rk-reg-shoot30 novice rookie regular 600 "opp.shoot_extra_m=30"
run T1-rk-large-shoot30 novice rookie large 600 "opp.shoot_extra_m=30"
