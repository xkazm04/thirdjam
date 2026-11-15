#!/bin/sh
# lane A206, build w2a: the proof (brain9=0, 100 a cell, six cells, vs A-*), then the six cells at 600 as shipped
cd "$(dirname "$0")"
echo "chain10 $$" >> out/pids.txt
W=2 ./cells.sh Pw2a 100 "brain9=0" "rk pro as"
for c in rk-reg rk-large pro-reg pro-large as-reg as-large; do echo "$c $(node same.mjs A-$c Pw2a-$c)"; done > out/proofw2a.txt
W=2 ./cells.sh Fw2a 600 "" "rk pro as"
echo chain10-done >> out/pids.txt
