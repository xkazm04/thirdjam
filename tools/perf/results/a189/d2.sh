#!/bin/sh
# lane A189 step 3: pro large red skill (large only) 1.18 / 1.21 (600, closing)
cd "$(dirname "$0")"
echo "d2 $$" > ../../../../Game/logs/a189/chain.pid
for r in 1.18 1.21; do node curve.mjs N-pro-large-red$r regular pro large 600 "red_skill=$r" --workers 3 --check closing > /dev/null; done
echo d2-done
