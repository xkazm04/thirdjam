#!/bin/sh
# lane A202: on build E (the final tree, all four on): F8-* as shipped (the default), the five cells F8-pro-reg (build D) did not cover
cd "$(dirname "$0")"
echo "chain-e $$" >> out/pids.txt
node curve.mjs F8-pro-large regular pro large 600 "" --workers 2 --check closing > /dev/null
node curve.mjs F8-as-reg good all_star regular 600 "" --workers 2 --check closing > /dev/null
node curve.mjs F8-as-large good all_star large 600 "" --workers 2 --check closing > /dev/null
node curve.mjs F8-rk-reg novice rookie regular 600 "" --workers 2 --check closing > /dev/null
node curve.mjs F8-rk-large novice rookie large 600 "" --workers 2 --check closing > /dev/null
echo echain-done
