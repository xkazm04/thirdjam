#!/bin/sh
# lane A206, build 9d: rookie's cells at 600 as shipped, then the proof on all six cells (brain9=0, 100 a cell, against A205's A-*)
cd "$(dirname "$0")"
echo "chain8 $$" >> out/pids.txt
W=2 ./cells.sh F9d 600 "" "rk"
W=2 ./cells.sh P9d 100 "brain9=0" "rk pro as"
for c in rk-reg rk-large pro-reg pro-large as-reg as-large; do echo "$c $(node same.mjs A-$c P9d-$c)"; done > out/proof9d.txt
echo chain8-done >> out/pids.txt
