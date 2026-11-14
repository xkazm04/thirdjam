#!/bin/sh
# lane A189: the unchanged cells at 2,000 as built (three workers): all_star regular, pro regular
cd "$(dirname "$0")"
echo "c4 $$" > ../../../../Game/logs/a189/chain.pid
:
node curve.mjs S1-as-reg good all_star regular 2000 --workers 2 --check closing > /dev/null
node curve.mjs S1-pro-reg regular pro regular 2000 --workers 2 --check closing > /dev/null
wait
cat ../../../../Game/logs/a189/law3-1.txt
echo c4-done
