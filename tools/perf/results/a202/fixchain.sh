#!/bin/sh
# lane A202: item 2, each fix alone, six cells x 600 paired seeds (labels FB-*, FC-*, FD-*), after the brace chain
cd "$(dirname "$0")"
echo "chain-fix $$" >> out/pids.txt
while ! grep -q brace-done out/brace.log; do sleep 15; done
W=2 ./cells.sh FC 600 "opp.hk_own_zone_turn=1;mate.hk_own_zone_turn=1"
W=2 ./cells.sh FB 600 "opp.hk_long_pass_m=30;mate.hk_long_pass_m=30"
W=2 ./cells.sh FD 600 "opp.hk_clump_frames=45;mate.hk_clump_frames=45"
echo fix-done
