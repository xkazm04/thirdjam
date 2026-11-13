#!/bin/sh
# Lane A86: the rest of the baseline on the built default (brain on the carrier reach, no env): six off-target cells, then a check that the built rule replays the env arm.
cd "$(dirname "$0")/../../../.."
D=tools/perf/results/a86/curve.mjs
for c in "regular rookie" "good rookie" "novice pro" "good pro" "novice all_star" "regular all_star"; do set -- $c
  node $D b-$1-$2 $1 $2 regular 2000 ""
done
node $D v-novice-rookie novice rookie regular 2000 ""
