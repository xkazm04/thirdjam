#!/bin/sh
# lane A170 item 3: everything switched off, on together (contact set + half-second screen + the mates' keep-out), six cells at 600, closing press, three workers.
cd "$(dirname "$0")"
SET="contact.downed_radius_m=0.6;contact.check_cooldown_frames=60;contact.check_cooldown_restart=true;contact.lunge_frames=12;contact.lunge_step_mps=1.5;contact.board_slam_frames=30"
T="mate.hk_screen_frames=30;mate.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_out=0.7"
for c in "E-rk-reg novice rookie regular" "E-pro-reg regular pro regular" "E-as-reg good all_star regular" "E-rk-large novice rookie large" "E-pro-large regular pro large" "E-as-large good all_star large"; do
  set -- $c; node curve.mjs $1 $2 $3 $4 600 "$T" --set "$SET" --workers 3 --check closing > /dev/null
done
