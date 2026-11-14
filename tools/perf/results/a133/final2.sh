#!/bin/sh
# lane A133: build 4 (pro large red 1.12): pro large as built at 2,000; all_star regular candidates at 2,000 (tunes)
cd "$(dirname "$0")"
run() { node curve.mjs "$@" --workers 3 > /dev/null; }
run F2-pro-large regular pro large 2000 &
run F2-as-reg-lift20 good all_star regular 2000 "opp.lift_cooldown_frames=20" &
run F2-as-reg-press015 good all_star regular 2000 "opp.hk_pressure_s=0.15" &
run F2-as-reg-red095 good all_star regular 2000 "red_skill=0.95" &
wait
