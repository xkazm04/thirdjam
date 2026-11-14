#!/bin/sh
# lane A201: the same 100 seeds a level on this worktree's build -> raw/fin2-<level>.jsonl; then same.mjs against main's
cd "$(dirname "$0")"
echo "hash-fin2 $$" >> out/pids.txt
for l in rookie pro all_star; do node curve.mjs fin2-$l regular $l regular 100 --check closing --workers 2; node same.mjs main-$l fin2-$l; done
echo hash-fin22-done
