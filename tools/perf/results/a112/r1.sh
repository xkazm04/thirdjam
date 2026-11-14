#!/bin/sh
# lane A112 round 1: aggression levers one at a time, hockey.regular, 600 a cell (base 99.8 / 72.0 / 39.8, A102)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
run L1-rk-poke20 novice rookie regular 600 "opp.poke_frames=20"
run L1-pro-hunt14 regular pro regular 600 "opp.hunt_radius_m=14"
run L1-as-hunt30 good all_star regular 600 "opp.hunt_radius_m=30"
run L1-rk-check4 novice rookie regular 600 "opp.check_min_closing=4"
run L1-pro-check1 regular pro regular 600 "opp.check_min_closing=1"
run L1-as-check1 good all_star regular 600 "opp.check_min_closing=1"
run L1-rk-poke10-check4 novice rookie regular 600 "opp.poke_frames=10;opp.check_min_closing=4"
run L1-pro-sprint07 regular pro regular 600 "opp.sprint_share=0.7"
run L1-pro-poke2 regular pro regular 600 "opp.poke_frames=2"
run L1-rk-hunt6 novice rookie regular 600 "opp.hunt_radius_m=6"
