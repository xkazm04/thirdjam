#!/bin/sh
# lane A206, build 9c: rookie's two cells at 600 as shipped (the other four are 9a's F9 rows: 9c changes rookie's profile only), then the rookie proof cells again
cd "$(dirname "$0")"
echo "chain6 $$" >> out/pids.txt
W=2 ./cells.sh F9c 600 "" "rk"
W=2 ./cells.sh P9c 100 "brain9=0" "rk"
for c in rk-reg rk-large; do echo "$c $(node same.mjs A-$c P9c-$c)"; done > out/proof9c.txt
echo chain6-done >> out/pids.txt
