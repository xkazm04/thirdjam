#!/bin/sh
# lane A206, build m5: six cells x 600 as shipped, then brain9=0 x 100 a cell (P2-*)
cd "$(dirname "$0")"
echo "chain12 $$" >> out/pids.txt
W=2 ./cells.sh F2 600 "" "rk pro as"
W=2 ./cells.sh P2 100 "brain9=0" "rk pro as"
echo chain12-done >> out/pids.txt
