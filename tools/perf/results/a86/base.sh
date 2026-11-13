#!/bin/sh
cd "$(dirname "$0")/../../../.."
D=tools/perf/results/a86/curve.mjs
node $D b-novice-rookie novice rookie regular 2000 "" --brain reach
node $D b-regular-pro regular pro regular 2000 "" --brain reach
node $D b-good-allstar good all_star regular 2000 "" --brain reach
node $D bL-novice-rookie novice rookie large 2000 "" --brain reach
node $D bL-regular-pro regular pro large 2000 "" --brain reach
node $D bL-good-allstar good all_star large 2000 "" --brain reach
for c in "regular rookie" "good rookie" "novice pro" "good pro" "novice all_star" "regular all_star"; do set -- $c
  node $D b-$1-$2 $1 $2 regular 2000 "" --brain reach
done
