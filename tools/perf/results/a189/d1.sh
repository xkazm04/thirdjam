#!/bin/sh
# lane A189 step 3 on main 58af359f (600, closing): rookie with its two numbers by tune (regular: red 0.80 + reach 20; large: reach 20), then the six cells as built
cd "$(dirname "$0")"
echo "d1 $$" > ../../../../Game/logs/a189/chain.pid
node curve.mjs N-rk-reg-tune novice rookie regular 600 "red_skill=0.80;opp.shoot_extra_m=20" --workers 3 --check closing > /dev/null
node curve.mjs N-rk-large-tune novice rookie large 600 "opp.shoot_extra_m=20" --workers 3 --check closing > /dev/null
for c in "N-pro-reg regular pro regular" "N-as-reg good all_star regular" "N-pro-large regular pro large" "N-as-large good all_star large" "N-rk-reg novice rookie regular" "N-rk-large novice rookie large"; do
  set -- $c; node curve.mjs $1 $2 $3 $4 600 --workers 3 --check closing > /dev/null
done
echo d1-done
