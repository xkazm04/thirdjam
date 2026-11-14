#!/bin/sh
# lane A202: the hash proof on build E (the final tree): S8-* brain8=0, 100 a cell, six cells, one worker, against main's B-*
cd "$(dirname "$0")"
echo "chain-proofE $$" >> out/pids.txt
W=1 ./cells.sh S8 100 "brain8=0"
for c in pro-reg as-reg rk-reg pro-large as-large rk-large; do node same.mjs B-$c S8-$c; done
echo proofE-done
