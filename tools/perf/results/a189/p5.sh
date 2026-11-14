#!/bin/sh
# lane A189 pass 1, rookie regular (600, closing): red 0.80 + shot reach +20; red 0.85 + reach +15
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a189/chain.pid
node curve.mjs R-rk-reg-red080-shoot20 novice rookie regular 600 "red_skill=0.80;opp.shoot_extra_m=20" --workers 3 --check closing > /dev/null
node curve.mjs R-rk-reg-red085-shoot15 novice rookie regular 600 "red_skill=0.85;opp.shoot_extra_m=15" --workers 3 --check closing > /dev/null
echo p5-done
