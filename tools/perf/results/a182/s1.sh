#!/bin/sh
# lane A182: main 9e452219 + the take shield (a177, neutral by default). 1. the default pro cell vs A162's rows (100); 2. everything on, six cells at 600 (closing);
# 3. one number a level with everything on: pro hunt 12/13/14 (with the presser keys), rookie red 1.0 / 1.03. Three workers, one chain.
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a182/chain.pid
R="$(pwd -W)/out/reg-set.cartridge.json"; L="$(pwd -W)/out/large-set.cartridge.json"
OLDSET="contact.downed_radius_m=0.6;contact.check_cooldown_frames=60;contact.check_cooldown_restart=true;contact.lunge_frames=12;contact.lunge_step_mps=1.5;contact.board_slam_frames=30;contact.take_shield_frames=15;contact.take_shield_stagger_max=45"
T="mate.hk_screen_frames=30;mate.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_out=0.7"
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
for c in "E-rk-reg novice rookie regular" "E-pro-reg regular pro regular" "E-as-reg good all_star regular" "E-rk-large novice rookie large" "E-pro-large regular pro large" "E-as-large good all_star large"; do
  set -- $c; V=$R; [ $4 = large ] && V=$L; node curve.mjs $1 $2 $3 $4 600 "$T" --cart "$V" --workers 3 --check closing > /dev/null
done
for h in 13 12 14; do node curve.mjs X-pro-reg-h$h regular pro regular 600 "$T;$K;opp.hunt_radius_m=$h" --cart "$R" --workers 3 --check closing > /dev/null; done
for r in 1.0 1.03; do node curve.mjs X-rk-reg-red$r novice rookie regular 600 "$T;red_skill=$r" --cart "$R" --workers 3 --check closing > /dev/null; done
echo s1-done
