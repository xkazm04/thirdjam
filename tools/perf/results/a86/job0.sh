#!/bin/sh
# Lane A86 job 0: the three target cells, the real stick (aim) and the old rest stick, n 2000 on hockey.regular.
cd "$(dirname "$0")/../../../.."
D=tools/perf/results/a86/curve.mjs
for st in aim rest; do
  x=""; [ $st = rest ] && x="--stick rest"
  node $D j0-$st-novice-rookie novice rookie regular 2000 "" $x
  node $D j0-$st-regular-pro regular pro regular 2000 "" $x
  node $D j0-$st-good-allstar good all_star regular 2000 "" $x
done
