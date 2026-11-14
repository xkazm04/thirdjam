#!/bin/sh
# lane A202: on build D: the gate (clean env, Jam), then law 3 (200 seeds a number) as shipped and with brain8=0 at pro and all_star, then the watch as shipped (w4)
cd "$(dirname "$0")"
echo "chain-law $$" >> out/pids.txt
./jam.sh gateD Jam
JAM_LAW3_AT="8,8.5,9,9.5" ./law3.sh law3D "pro:-|pro:brain8=0|all_star:-|all_star:brain8=0"
./watch.sh w4 "seeds=6"
echo lawchain-done
