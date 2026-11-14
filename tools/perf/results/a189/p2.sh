#!/bin/sh
# lane A189 pass 1, rookie's second number with red 0.80 (600, closing): poke after 14 frames; shot reach + 15; speed 0.80
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a189/chain.pid
node curve.mjs R-rk-reg-red080-poke14 novice rookie regular 600 "red_skill=0.80;opp.poke_frames=14" --workers 3 --check closing > /dev/null
node curve.mjs R-rk-reg-red080-shoot15 novice rookie regular 600 "red_skill=0.80;opp.shoot_extra_m=15" --workers 3 --check closing > /dev/null
node curve.mjs R-rk-reg-red080-sp080 novice rookie regular 600 "red_skill=0.80;speed_scale=0.80" --workers 3 --check closing > /dev/null
echo p2-done
