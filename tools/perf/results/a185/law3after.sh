#!/bin/sh
# lane A185: law 3 on the built tree, the shipped rule and the neutral arm, 7 distances, three levels; then the cells (after.sh)
cd "$(dirname "$0")"
echo "chain-law3after $$" >> out/pids.txt
AT="7,8,8.5,9,9.5,10,11,12"
JAM_LAW3_AT="$AT" sh law3.sh law3-after "rookie:-|pro:-|all_star:-"
JAM_LAW3_AT="$AT" JAM_LAW3_FREE=1 sh law3.sh law3-neutral "rookie:-|pro:-|all_star:-"
echo law3-done
sh after.sh
