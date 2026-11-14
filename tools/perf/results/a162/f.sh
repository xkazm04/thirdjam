#!/bin/sh
# lane A162: one chain, three workers: all_star regular at 2,000 (as built, hk_pressure_s 0.2), red 1.0 at 600, then the other five cells at 2,000
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 --check closing > /dev/null; }
run F-as-reg good all_star regular 2000
run F-as-reg-hkp02 good all_star regular 2000 "opp.hk_pressure_s=0.2"
run T4-as-reg-red10 good all_star regular 600 "red_skill=1.0"
run F-rk-large novice rookie large 2000
run F-pro-reg regular pro regular 2000
run F-as-large good all_star large 2000
run F-pro-large regular pro large 2000
run F-rk-reg novice rookie regular 2000
