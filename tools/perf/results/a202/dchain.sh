#!/bin/sh
# lane A202: on build D (the flip: all four ship on). 1) the hash proof R8-* (brain8=0, 100 a cell, six cells) against main's B-*;
# 2) F8-* (as shipped = all four, 600 a cell); law 3 runs beside F8 (lawchain.sh). Child pids in out/pids.txt.
cd "$(dirname "$0")"
echo "chain-d $$" >> out/pids.txt
W=2 ./cells.sh R8 100 "brain8=0"
for c in pro-reg as-reg rk-reg pro-large as-large rk-large; do node same.mjs B-$c R8-$c; done
echo proofD-done
W=2 ./cells.sh F8 600 ""
echo dchain-done
