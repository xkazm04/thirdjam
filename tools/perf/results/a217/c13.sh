#!/bin/sh
# lane A217: pro round 2 on build 6 (100 a point, four workers), after c12: the shot reach 20 (far goals 2 %) with the red skill.
cd "$(dirname "$0")"
echo "c13 $$ $(date +%H:%M)" >> out/pids.txt
until grep -q 'c12 done' out/pids.txt 2>/dev/null; do sleep 5; done
p() { node hand.mjs "$1" pro "$2" 100 --tune "$3" --workers 4 > /dev/null; }
p E-pro-reg-sx20-red105 regular "opp.shoot_extra_m=20;red_skill=1.05"
p E-pro-reg-sx20-red110 regular "opp.shoot_extra_m=20;red_skill=1.10"
p E-pro-large-sx20-red100 large "opp.shoot_extra_m=20;red_skill=1.00"
echo "c13 done $(date +%H:%M)" >> out/pids.txt
