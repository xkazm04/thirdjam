#!/bin/sh
# lane A193: on build 4 (the final tree): the hash proofs, 100 seeds a cell. N4-*: brain7=0 against build 0 (B-*); R4-rk-*: rookie as shipped against build 0
# (rookie keeps the old planning); V4-*: pro and all_star as shipped against build 3's S-* (the same rule). Child pids in out/pids.txt.
cd "$(dirname "$0")"
echo "chain-final $$" >> out/pids.txt
for c in "pro-reg regular pro regular" "as-reg good all_star regular" "rk-reg novice rookie regular" "as-large good all_star large" "pro-large regular pro large" "rk-large novice rookie large"; do
  set -- $c; node curve.mjs N4-$1 $2 $3 $4 100 "brain7=0" --workers ${W:-2} --check closing > /dev/null
done
for c in "rk-reg novice rookie regular" "rk-large novice rookie large"; do set -- $c; node curve.mjs R4-$1 $2 $3 $4 100 --workers ${W:-2} --check closing > /dev/null; done
for c in "pro-reg regular pro regular" "as-reg good all_star regular" "pro-large regular pro large" "as-large good all_star large"; do set -- $c; node curve.mjs V4-$1 $2 $3 $4 100 --workers ${W:-2} --check closing > /dev/null; done
for c in pro-reg as-reg rk-reg as-large pro-large rk-large; do node same.mjs B-$c N4-$c; done
for c in rk-reg rk-large; do node same.mjs B-$c R4-$c; done
for c in pro-reg as-reg pro-large as-large; do node same.mjs S-$c V4-$c; done
echo final-done
