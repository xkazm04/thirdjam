#!/bin/sh
# lane A217 rookie candidates on build 5, 100 a point, 3 workers, after c10. Regular: the pace floor (hk_opp_pace9) and the poke; large: its own red.
cd "$(dirname "$0")"
echo "c11 $$ $(date +%H:%M)" >> out/pids.txt
until grep -q 'c10 done' out/pids.txt 2>/dev/null; do sleep 10; done
p() { node hand.mjs "$1" rookie "$2" 100 --tune "$3" --workers 3 > /dev/null; }
p Q-rk-reg-pace095 regular "opp.hk_opp_pace9=0.95"
p Q-rk-reg-pace100 regular "opp.hk_opp_pace9=1.0"
p Q-rk-reg-poke10 regular "opp.hk_poke9_frames=10"
p Q-rk-reg-pace095-poke10 regular "opp.hk_opp_pace9=0.95;opp.hk_poke9_frames=10"
p Q-rk-large-red117 large "red_skill=1.17"
p Q-rk-large-red122 large "red_skill=1.22"
echo "c11 done $(date +%H:%M)" >> out/pids.txt
