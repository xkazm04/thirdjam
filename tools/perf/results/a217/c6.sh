#!/bin/sh
# lane A217: ROOKIE candidates (Director 05:08), 100 seeds a point, person's path, after c5. Clumsy never absent: only execution numbers.
cd "$(dirname "$0")"
echo "c6 $$ $(date +%H:%M)" >> out/pids.txt
until grep -q '"label":"C-as-large"' cells.jsonl 2>/dev/null; do sleep 10; done
p() { node hand.mjs "$1" rookie "$2" 100 --tune "$3" --workers 2 > /dev/null; }
p K-rk-reg-aim4 regular "opp.hk_aim_error9_deg=4"
p K-rk-reg-aim3 regular "opp.hk_aim_error9_deg=3"
p K-rk-reg-sx13 regular "opp.hk_shoot_extra9_m=13"
p K-rk-reg-pace092 regular "opp.hk_opp_pace9=0.92"
p K-rk-reg-pace092-aim4 regular "opp.hk_opp_pace9=0.92;opp.hk_aim_error9_deg=4"
p K-rk-large-aim4 large "opp.hk_aim_error9_deg=4"
p K-rk-large-aim3 large "opp.hk_aim_error9_deg=3"
p K-rk-large-red114 large "red_skill=1.14"
echo "c6 done $(date +%H:%M)" >> out/pids.txt
