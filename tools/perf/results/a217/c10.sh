#!/bin/sh
# lane A217 LADDER SET 3 candidates on build 5 (wave 4 + CROSS), 100 a point, 3 workers, after c8. Pro regular is at 46 as built: pro large by its own red only.
cd "$(dirname "$0")"
echo "c10 $$ $(date +%H:%M)" >> out/pids.txt
until grep -q 'c8 done' out/pids.txt 2>/dev/null; do sleep 10; done
p() { node hand.mjs "$1" "$2" "$3" 100 --tune "$4" --workers 3 > /dev/null; }
p Q-pro-large-red110 pro large "red_skill=1.10"
p Q-pro-large-red105 pro large "red_skill=1.05"
p Q-pro-large-red100 pro large "red_skill=1.00"
p Q-as-reg-dodge02 all_star regular "opp.hk_dodge9_share=0.2"
p Q-as-reg-poke6 all_star regular "opp.poke_frames=6"
p Q-as-reg-react6 all_star regular "opp.reaction_frames=6"
p Q-as-reg-aim6 all_star regular "opp.aim_error_deg=6"
p Q-as-reg-speed135 all_star regular "speed_scale=1.35"
p Q-as-reg-mix all_star regular "opp.hk_dodge9_share=0.2;opp.poke_frames=6;opp.aim_error_deg=6"
echo "c10 done $(date +%H:%M)" >> out/pids.txt
