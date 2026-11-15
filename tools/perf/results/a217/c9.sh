#!/bin/sh
# lane A217 LADDER SET 3 candidates, pro and all_star (100 a point, 3 workers), after c8. Execution only: the shot's reach, the far shot's aim, the red skill, the dodge.
cd "$(dirname "$0")"
echo "c9 $$ $(date +%H:%M)" >> out/pids.txt
until grep -q 'c8 done' out/pids.txt 2>/dev/null; do sleep 10; done
p() { node hand.mjs "$1" "$2" "$3" 100 --tune "$4" --workers 3 > /dev/null; }
p Q-pro-reg-sx20 pro regular "opp.shoot_extra_m=20"
p Q-pro-reg-sx15 pro regular "opp.shoot_extra_m=15"
p Q-pro-reg-aim6 pro regular "opp.aim_error_deg=6"
p Q-pro-reg-red095 pro regular "red_skill=0.95"
p Q-pro-large-sx20 pro large "opp.shoot_extra_m=20"
p Q-pro-large-sx15 pro large "opp.shoot_extra_m=15"
p Q-pro-large-red110 pro large "red_skill=1.10"
p Q-as-reg-sx30 all_star regular "opp.shoot_extra_m=30"
p Q-as-reg-dodge02 all_star regular "opp.hk_dodge9_share=0.2"
p Q-as-reg-aim6 all_star regular "opp.aim_error_deg=6"
p Q-as-large-sx30 all_star large "opp.shoot_extra_m=30"
echo "c9 done $(date +%H:%M)" >> out/pids.txt
