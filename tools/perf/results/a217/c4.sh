#!/bin/sh
# lane A217 CURVE SLOPES (Director 04:42): 100 seeds a point through the person's path (hand.mjs), two workers, build 2 (-aitune).
cd "$(dirname "$0")"
echo "c4 $$ $(date +%H:%M)" >> out/pids.txt
FAR="opp.shoot_extra_m=18.5;opp.hk_shoot_extra9_m=18.5"
HUNT="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8;opp.hunt_radius_m="
p() { node hand.mjs "$1" "$2" "$3" 100 --tune "$4" --workers 2 > /dev/null; }
p S-pro-reg-far pro regular "$FAR"
p S-pro-large-far pro large "$FAR"
p S-as-reg-far all_star regular "$FAR"
p S-pro-reg-pr10 pro regular "opp.hk_pressure_s=1.0"
p S-pro-reg-pr14 pro regular "opp.hk_pressure_s=1.4"
p S-rk-reg-red085 rookie regular "red_skill=0.85"
p S-rk-reg-red090 rookie regular "red_skill=0.90"
p S-rk-reg-red100 rookie regular "red_skill=1.0"
p S-rk-reg-sx15 rookie regular "opp.hk_shoot_extra9_m=15"
p S-rk-reg-sx25 rookie regular "opp.hk_shoot_extra9_m=25"
p S-pro-large-pr10 pro large "opp.hk_pressure_s=1.0"
p S-pro-large-pr14 pro large "opp.hk_pressure_s=1.4"
p S-pro-large-red110 pro large "red_skill=1.10"
p S-pro-large-red130 pro large "red_skill=1.13"
p S-pro-large-red150 pro large "red_skill=1.15"
for h in 10 11 12; do p S-pro-reg-hunt$h pro regular "$HUNT$h"; done
for h in 10 11 12; do p S-pro-large-hunt$h pro large "$HUNT$h"; done
echo "c4 done $(date +%H:%M)" >> out/pids.txt
