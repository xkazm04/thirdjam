#!/bin/sh
# lane A207: the default match (regular player, the ladder's closing press) on 100 seeds a level and rink on this worktree's PRE-EDIT build (b530b4eb) -> raw/base-<level>-<rink>.jsonl
cd "$(dirname "$0")"
echo "base.sh $$" >> out/pids.txt
for r in regular large; do for l in rookie pro all_star; do node curve.mjs base-$l-$r regular $l $r 100 --check closing --workers 3; done; done
echo base-done
