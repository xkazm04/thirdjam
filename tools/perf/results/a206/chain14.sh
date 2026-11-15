#!/bin/sh
# lane A206, build m6: six cells x 600 as shipped
cd "$(dirname "$0")"
echo "chain14 $$" >> out/pids.txt
W=2 ./cells.sh F3 600 "" "rk pro as"
echo chain14-done >> out/pids.txt
