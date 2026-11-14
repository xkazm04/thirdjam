#!/bin/sh
# lane A207: the six ladder cells (600 matches each, regular player, closing press) on this tree as it ships -> cells.jsonl rows on-<level>-<rink>
cd "$(dirname "$0")"
echo "cells.sh $$" >> out/pids.txt
for r in regular large; do for l in rookie pro all_star; do node curve.mjs on-$l-$r regular $l $r 600 --check closing --workers ${W:-2} > /dev/null; done; done
echo cells-done
