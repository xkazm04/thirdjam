#!/bin/sh
# lane A206, build 9a: the hash proof (brain9=0, 100 seeds a cell, six cells, against lane A205's A-* rows on the H34 game), then the six cells at 600 as shipped
cd "$(dirname "$0")"
echo "chain2 $$" >> out/pids.txt
W=2 ./cells.sh P9 100 "brain9=0"
for c in pro-reg pro-large as-reg as-large rk-reg rk-large; do echo "$c $(node same.mjs A-$c P9-$c)"; done > out/proof9a.txt
W=2 ./cells.sh F9 600 ""
echo chain2-done >> out/pids.txt
