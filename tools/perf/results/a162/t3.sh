#!/bin/sh
# lane A162: all_star regular, opp.hk_pressure_s down (0.6 read 37.2 against 29.8 at 0.3: the carrier that carries through the forecheck beats the human); 600, tunes
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 --check closing > /dev/null; }
run T3-as-reg-hkp02 good all_star regular 600 "opp.hk_pressure_s=0.2"
run T3-as-reg-hkp01 good all_star regular 600 "opp.hk_pressure_s=0.1"
