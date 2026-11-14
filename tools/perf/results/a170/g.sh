#!/bin/sh
# lane A170 item 2: the player who lands his checks (JAM_PLAYTESTER_CHECK unset: the course press, as the config ships; A162's P-pro-reg 56.0 at hunt 8) on the same dial,
# then the whiffing player (closing) at the high end; 600 a point, three workers, one chain
cd "$(dirname "$0")"
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
for h in 14 16 12; do node curve.mjs G-pro-reg-course-h$h regular pro regular 600 "$K;opp.hunt_radius_m=$h" --workers 3 > /dev/null; done
for h in 16; do node curve.mjs G-pro-reg-closing-h$h regular pro regular 600 "$K;opp.hunt_radius_m=$h" --workers 3 --check closing > /dev/null; done
