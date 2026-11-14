#!/bin/sh
# lane A206: rookie regular (novice, 200) arms, round 2
cd "$(dirname "$0")"
echo "sweep2 $$" >> out/pids.txt
run() { node curve.mjs "$1" novice rookie regular 200 "$2" --workers 1 --check closing > /dev/null; }
run R2-a "opp.hk_opp_pace9=0.9;opp.poke_frames=20;opp.hk_aim_error9_deg=6;opp.shoot_extra_m=10" &
run R2-b "opp.hk_opp_pace9=0.9;opp.poke_frames=30;opp.hk_aim_error9_deg=8;opp.shoot_extra_m=8" &
run R2-c "opp.hk_opp_pace9=0.9;opp.poke_frames=20;opp.hk_aim_error9_deg=6;opp.shoot_extra_m=10;opp.check_min_closing=-1" &
wait
echo sweep2-done >> out/pids.txt
