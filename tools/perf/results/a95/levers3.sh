#!/bin/sh
# lane A95 job 4, round 3: pro's shot execution and pass timing; all_star speed and the hunt; the rookie combination; 600 per cell
cd "$(dirname "$0")"
run() { node curve.mjs "$@" > /dev/null; }
run L3-rk-combo novice rookie regular 600 "red_skill=0.9;opp.hk_share=0.85;opp.hk_late_frames=8;opp.check_min_closing=6;opp.sprint_share=0.3"
run L3-pro-shoot30 regular pro regular 600 "opp.shoot_extra_m=30"
run L3-pro-press20 regular pro regular 600 "opp.hk_pressure_s=2.0"
run L3-pro-charge18 regular pro regular 600 "opp.shot_charge_frames=18"
run L3-as-speed135 good all_star regular 600 "speed_scale=1.35"
run L3-as-huntsprint good all_star regular 600 "opp.hunt_radius_m=14;opp.sprint_share=1"
run L3-as-press20 good all_star regular 600 "opp.hk_pressure_s=2.0"
