#!/bin/sh
# lane A202: on build C: the Hockey8 tests, then the watch arms (wB2: the long pass reworked; w3: brain8=1, every rule at its final value)
cd "$(dirname "$0")"
echo "chain-w3 $$" >> out/pids.txt
./jam.sh t8c Jam.Bot.Hockey8
./watch.sh wB2 "seeds=6;tune=opp.hk_long_pass_m=30|mate.hk_long_pass_m=30"
./watch.sh w3 "seeds=6;tune=brain8=1"
echo wchain3-done
