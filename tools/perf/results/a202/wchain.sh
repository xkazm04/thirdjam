#!/bin/sh
# lane A202: the watch's arms and dumps, one process at a time. pids in Game/logs/a202/<tag>.pid
cd "$(dirname "$0")"
echo "chain-watch $$" >> out/pids.txt
./watch.sh w2 "seeds=6;tune=brain8=1"
./watch.sh d1 "carts=hockey.arena.large;levels=all_star;seeds=1;from=5;dump=1830:2270:15"
./watch.sh d2 "carts=hockey.arena.large;levels=all_star;seeds=1;from=4;dump=7170:7550:15"
./watch.sh d3 "carts=hockey.arena.large;levels=pro;seeds=1;from=5;dump=9220:9730:30"
./watch.sh d4 "carts=hockey.arena.large;levels=all_star;seeds=1;from=3;dump=9380:9530:5"
echo wchain-done
