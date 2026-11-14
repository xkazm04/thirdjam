#!/bin/sh
# lane A202: the hash proofs on build C, 100 seeds a cell, six cells, one worker, after redo.sh: P8-* brain8=0, Q8-* brain8 at its default, against main's B-*
cd "$(dirname "$0")"
echo "chain-proof $$" >> out/pids.txt
while ! grep -q redo-done out/redo.log; do sleep 15; done
W=1 ./cells.sh P8 100 "brain8=0"
W=1 ./cells.sh Q8 100 ""
for c in pro-reg as-reg rk-reg pro-large as-large rk-large; do node same.mjs B-$c P8-$c; node same.mjs B-$c Q8-$c; done
echo proof-done
