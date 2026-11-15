#!/bin/sh
# lane A217: ROOKIE candidates, second round (after PASS FATES), 100 seeds a point. Clumsy never absent: execution numbers only; the poke goes DOWN (law 3's straight charge is safe below 14).
cd "$(dirname "$0")"
echo "c6b $$ $(date +%H:%M)" >> out/pids.txt
until grep -q 'fates done' out/pids.txt 2>/dev/null; do sleep 10; done
p() { node hand.mjs "$1" rookie "$2" 100 --tune "$3" --workers 2 > /dev/null; }
p K-rk-reg-poke12 regular "opp.hk_poke9_frames=12"
p K-rk-reg-poke10 regular "opp.hk_poke9_frames=10"
p K-rk-reg-pace095 regular "opp.hk_opp_pace9=0.95"
p K-rk-reg-sx16 regular "opp.hk_shoot_extra9_m=16"
p K-rk-reg-pace095-poke12 regular "opp.hk_opp_pace9=0.95;opp.hk_poke9_frames=12"
p K-rk-large-poke12 large "opp.hk_poke9_frames=12"
p K-rk-large-red114 large "red_skill=1.14"
p K-rk-large-red117 large "red_skill=1.17"
echo "c6b done $(date +%H:%M)" >> out/pids.txt
