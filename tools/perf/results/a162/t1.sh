#!/bin/sh
# lane A162: all_star regular trims at 600 (tunes on build 1), aggression first, speed last; one at a time, three workers
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 --check closing > /dev/null; }
run T1-as-reg-cmc10 good all_star regular 600 "opp.check_min_closing=1.0"
run T1-as-reg-cmc05 good all_star regular 600 "opp.check_min_closing=0.5"
run T1-as-reg-sp147 good all_star regular 600 "speed_scale=1.47"
