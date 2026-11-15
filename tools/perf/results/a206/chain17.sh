#!/bin/sh
# lane A206, build m8: six cells x 600 as shipped
cd "$(dirname "$0")"
echo "chain17 $$" >> out/pids.txt
W=2 ./cells.sh F5 600 "" "rk pro as"
echo chain17-done >> out/pids.txt
