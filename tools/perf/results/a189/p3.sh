#!/bin/sh
# lane A189 pass 1, all_star regular (600, closing): speed 1.5 with the brain dial; 1.52; 1.48 with the dial
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a189/chain.pid
D="opp.hk_second_man_hunts=1;opp.hk_step_up_m=16;opp.hk_step_up_max_team=99"
node curve.mjs A-as-reg-sp1.5-dial good all_star regular 600 "speed_scale=1.5;$D" --workers 3 --check closing > /dev/null
node curve.mjs A-as-reg-sp1.52 good all_star regular 600 "speed_scale=1.52" --workers 3 --check closing > /dev/null
node curve.mjs A-as-reg-sp1.48-dial good all_star regular 600 "speed_scale=1.48;$D" --workers 3 --check closing > /dev/null
echo p3-done
