#!/bin/sh
# lane A217: pro regular slopes on the applied tree of 05:08 (build 3), 100 a point, after c6.
cd "$(dirname "$0")"
echo "c7 $$ $(date +%H:%M)" >> out/pids.txt
until grep -q 'c6 done' out/pids.txt 2>/dev/null; do sleep 10; done
p() { node hand.mjs "$1" pro regular 100 --tune "$2" --workers 2 > /dev/null; }
p P-pro-reg-pr14 "opp.hk_pressure_s=1.4"
p P-pro-reg-sx20 "opp.shoot_extra_m=20"
p P-pro-reg-hunt11 "opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8;opp.hunt_radius_m=11"
p P-pro-reg-red095 "red_skill=0.95"
p P-pro-reg-speed090 "speed_scale=0.9"
echo "c7 done $(date +%H:%M)" >> out/pids.txt
