#!/bin/sh
# lane A162: all_star regular trims at 600 (tunes), the levers that answer the poke; one at a time, three workers
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 --check closing > /dev/null; }
run T2-as-reg-hkp06 good all_star regular 600 "opp.hk_pressure_s=0.6"
run T2-as-reg-bb6 good all_star regular 600 "opp.hk_better_by_m=6"
run T2-as-reg-red08 good all_star regular 600 "red_skill=0.8"
