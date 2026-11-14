#!/bin/sh
# lane A206: rookie regular (novice, 200) arms for item 3: easing by execution, not by absence
cd "$(dirname "$0")"
echo "sweep1 $$" >> out/pids.txt
run() { node curve.mjs "$1" novice rookie regular 200 "$2" --workers 1 --check closing > /dev/null; }
run R1-pace90 "opp.hk_opp_pace9=0.9" &
run R1-poke20 "opp.poke_frames=20" &
run R1-aim6 "opp.hk_aim_error9_deg=6" &
wait
run R1-reach10 "opp.shoot_extra_m=10" &
run R1-combo "opp.hk_opp_pace9=0.9;opp.poke_frames=20;opp.hk_aim_error9_deg=6" &
run R1-pace85 "opp.hk_opp_pace9=0.85" &
wait
echo sweep1-done >> out/pids.txt
