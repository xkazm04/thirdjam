#!/bin/sh
# lane A217 LADDER SET 3 candidates on the closed sim (build 6), 100 a point, four workers. Execution only.
cd "$(dirname "$0")"
echo "c12 $$ $(date +%H:%M)" >> out/pids.txt
p() { node hand.mjs "$1" "$2" "$3" 100 ${4:+--tune "$4"} --workers 4 > /dev/null; }
SX20="opp.shoot_extra_m=20"
p E-pro-reg pro regular ""
p E-pro-reg-sx20 pro regular "$SX20"
p E-pro-reg-sx15 pro regular "opp.shoot_extra_m=15"
p E-pro-large pro large ""
p E-pro-large-sx20-red110 pro large "$SX20;red_skill=1.10"
p E-pro-large-sx20-red105 pro large "$SX20;red_skill=1.05"
p E-rk-reg rookie regular ""
p E-rk-reg-pace095 rookie regular "opp.hk_opp_pace9=0.95"
p E-rk-reg-pace100 rookie regular "opp.hk_opp_pace9=1.0"
p E-rk-large rookie large ""
p E-rk-large-red117 rookie large "red_skill=1.17"
p E-rk-large-red122 rookie large "red_skill=1.22"
MIX="opp.hk_dodge9_share=0.2;opp.poke_frames=6;opp.aim_error_deg=6"
p E-as-reg all_star regular ""
p E-as-reg-mix all_star regular "$MIX"
p E-as-reg-mix-sp135 all_star regular "$MIX;speed_scale=1.35"
p E-as-large-mix all_star large "$MIX"
p E-as-large-mix-sp137 all_star large "$MIX;speed_scale=1.37"
echo "c12 done $(date +%H:%M)" >> out/pids.txt
