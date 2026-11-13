#!/bin/sh
# lane A95 job 3: the playtester's verbs, before (JAM_PLAYTESTER_VERBS=old) and after, 600 matches per cell, paired seeds
cd "$(dirname "$0")"
for c in "novice rookie" "regular pro" "good all_star"; do
  set -- $c
  node curve.mjs j3-$1-old $1 $2 regular 600 --verbs old
  node curve.mjs j3-$1-new $1 $2 regular 600
done
