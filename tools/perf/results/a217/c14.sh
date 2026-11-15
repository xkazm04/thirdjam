#!/bin/sh
# lane A217: all_star large round 2 (100, four workers) after c13.
cd "$(dirname "$0")"
echo "c14 $$ $(date +%H:%M)" >> out/pids.txt
until grep -q 'c13 done' out/pids.txt 2>/dev/null; do sleep 5; done
MIX="opp.hk_dodge9_share=0.2;opp.poke_frames=6;opp.aim_error_deg=6"
node hand.mjs E-as-large-mix-sp125 all_star large 100 --tune "$MIX;speed_scale=1.25" --workers 4 > /dev/null
node hand.mjs E-as-large-mix-sp115 all_star large 100 --tune "$MIX;speed_scale=1.15" --workers 4 > /dev/null
echo "c14 done $(date +%H:%M)" >> out/pids.txt
