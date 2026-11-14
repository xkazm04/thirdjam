#!/bin/sh
# lane A112 round 4: pro and all_star: hunt + shot range + speed, hockey.regular, 600 a cell, own build
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 4 > /dev/null; }
run L4-pro-hunt14-shoot45 regular pro regular 600 "opp.hunt_radius_m=14;opp.shoot_extra_m=45"
run L4-pro-hunt14-red115 regular pro regular 600 "opp.hunt_radius_m=14;red_skill=1.15"
run L4-as-shoot60 good all_star regular 600 "opp.shoot_extra_m=60"
run L4-as-red10 good all_star regular 600 "opp.hunt_radius_m=30;red_skill=1.0"
