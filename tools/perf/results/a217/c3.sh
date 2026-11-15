#!/bin/sh
# lane A217: what makes the gap (100 seeds, pro and all_star regular): the game's path with the verbs (buttons 0), without the person hand (hand 0), without sprint (sprint 0).
cd "$(dirname "$0")"
echo "c3 $$ $(date +%H:%M)" >> out/pids.txt
until grep -q '"label":"V-as-large"' cells.jsonl 2>/dev/null; do sleep 10; done
node hand.mjs X-pro-reg-b0 pro regular 100 --buttons 0 --workers 2 > /dev/null
node hand.mjs X-pro-reg-h0 pro regular 100 --hand 0 --workers 2 > /dev/null
node hand.mjs X-pro-reg-s0 pro regular 100 --sprint 0 --workers 2 > /dev/null
node hand.mjs X-pro-reg-b0h0s0 pro regular 100 --buttons 0 --hand 0 --sprint 0 --workers 2 > /dev/null
node hand.mjs X-as-reg-b0 all_star regular 100 --buttons 0 --workers 2 > /dev/null
node hand.mjs X-rk-reg-b0 rookie regular 100 --buttons 0 --workers 2 > /dev/null
echo "c3 done $(date +%H:%M)" >> out/pids.txt
