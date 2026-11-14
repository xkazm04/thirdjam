#!/bin/sh
# lane A206, build 9b: rookie's two cells at 600 as shipped (the other four are 9a's F9 rows: 9b changes rookie's profile only), then the rookie proof cells again
cd "$(dirname "$0")"
echo "chain4 $$" >> out/pids.txt
W=2 ./cells.sh F9b 600 "" "rk"
W=2 ./cells.sh P9b 100 "brain9=0" "rk"
for c in rk-reg rk-large; do echo "$c $(node same.mjs A-$c P9b-$c)"; done > out/proof9b.txt
echo chain4-done >> out/pids.txt
