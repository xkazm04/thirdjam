#!/bin/sh
# lane A189 LADDER SET 2: the four unchanged cells at 2,000 with brain8=1 (three workers), as many as the clock allows
cd "$(dirname "$0")"
echo "h2 $$" > ../../../../Game/logs/a189/chain.pid
for c in "T-as-reg good all_star regular" "T-rk-reg novice rookie regular" "T-as-large good all_star large" "T-rk-large novice rookie large"; do
  H=$(date +%H%M); [ $H -ge 1802 ] && { echo "stop at $H"; break; }
  set -- $c; node curve.mjs $1 $2 $3 $4 2000 "brain8=1" --workers 3 --check closing > /dev/null
done
echo h2-done
