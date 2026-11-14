#!/bin/sh
cd "$(dirname "$0")"
echo $$ > ../../../../Game/logs/a189/chain.pid
node curve.mjs A-as-reg-sp1.48 good all_star regular 600 "speed_scale=1.48" --workers 3 --check closing > /dev/null
echo p6-done
