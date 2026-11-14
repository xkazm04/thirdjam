#!/bin/sh
# lane A170 item 3, second round (600, closing, everything on): pro hunt 12 on both rinks, all_star's pressure time 0.2 (A162's one lever that moved it at all)
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a170/chain.pid
SET="contact.downed_radius_m=0.6;contact.check_cooldown_frames=60;contact.check_cooldown_restart=true;contact.lunge_frames=12;contact.lunge_step_mps=1.5;contact.board_slam_frames=30"
T="mate.hk_screen_frames=30;mate.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_out=0.7"
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
node curve.mjs Y-pro-reg-h12 regular pro regular 600 "$T;$K;opp.hunt_radius_m=12" --set "$SET" --workers 3 --check closing > /dev/null
node curve.mjs Y-as-reg-hkp02 good all_star regular 600 "$T;opp.hk_pressure_s=0.2" --set "$SET" --workers 3 --check closing > /dev/null
node curve.mjs Y-pro-large-h12 regular pro large 600 "$T;$K;opp.hunt_radius_m=12" --set "$SET" --workers 3 --check closing > /dev/null
node curve.mjs Y-as-large-hkp02 good all_star large 600 "$T;opp.hk_pressure_s=0.2" --set "$SET" --workers 3 --check closing > /dev/null
node curve.mjs Y-rk-large-red115 novice rookie large 600 "$T;red_skill=1.15" --set "$SET" --workers 3 --check closing > /dev/null
node curve.mjs G2-pro-reg-course-h11 regular pro regular 2000 "$K;opp.hunt_radius_m=11" --workers 3 > /dev/null
echo y-done
