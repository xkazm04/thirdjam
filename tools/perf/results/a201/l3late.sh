#!/bin/sh
# lane A201 evolution 12: the late hand at pro by the line's near edge (JAM_MARK_RULE near=), one process a value
cd "$(dirname "$0")"
echo "l3late $$" >> out/pids.txt
for n in 7 7.5 8 8.5; do JAM_MARK_RULE="near=$n" JAM_LAW3_AT=8.5 sh law3.sh l3late-n$n "pro:-|all_star:-|rookie:-"; done
echo l3late-done
