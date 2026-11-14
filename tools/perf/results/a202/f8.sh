#!/bin/sh
# lane A202: every rule together (brain8=1), six cells x 600 (F8-*), after cchain
cd "$(dirname "$0")"
echo "chain-f8 $$" >> out/pids.txt
while ! grep -q cchain-done out/cchain.log; do sleep 15; done
W=2 ./cells.sh F8 600 "brain8=1"
echo f8-done
