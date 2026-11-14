#!/bin/sh
# lane A170 item 3: everything on + one number a level (600, closing), regular rink first; then item 2 at 2,000 (course press)
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a170/chain.pid
SET="contact.downed_radius_m=0.6;contact.check_cooldown_frames=60;contact.check_cooldown_restart=true;contact.lunge_frames=12;contact.lunge_step_mps=1.5;contact.board_slam_frames=30"
T="mate.hk_screen_frames=30;mate.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_out=0.7"
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
node curve.mjs X-pro-reg-h13 regular pro regular 600 "$T;$K;opp.hunt_radius_m=13" --set "$SET" --workers 3 --check closing > /dev/null
node curve.mjs X-as-reg-c8 good all_star regular 600 "$T;opp.hk_press_commit_m=8" --set "$SET" --workers 3 --check closing > /dev/null
node curve.mjs X-rk-reg-red100 novice rookie regular 600 "$T;red_skill=1.0" --set "$SET" --workers 3 --check closing > /dev/null
node curve.mjs X-as-reg-c6 good all_star regular 600 "$T;opp.hk_press_commit_m=6" --set "$SET" --workers 3 --check closing > /dev/null
node curve.mjs X-pro-large-h13 regular pro large 600 "$T;$K;opp.hunt_radius_m=13" --set "$SET" --workers 3 --check closing > /dev/null
node curve.mjs X-as-large-c8 good all_star large 600 "$T;opp.hk_press_commit_m=8" --set "$SET" --workers 3 --check closing > /dev/null
node curve.mjs X-rk-large-red116 novice rookie large 600 "$T;red_skill=1.16" --set "$SET" --workers 3 --check closing > /dev/null
node curve.mjs G2-pro-reg-course-h12 regular pro regular 2000 "$K;opp.hunt_radius_m=12" --workers 3 > /dev/null
node curve.mjs G2-pro-reg-course-h13 regular pro regular 2000 "$K;opp.hunt_radius_m=13" --workers 3 > /dev/null
echo x-done
