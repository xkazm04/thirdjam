#!/bin/sh
# lane A202: one rule at a time on the watch's 60 matches (before/after counts per fix), after wchain. pids in Game/logs/a202/<tag>.pid
cd "$(dirname "$0")"
echo "chain-watch2 $$" >> out/pids.txt
while grep -q "^" out/wchain.log && ! grep -q wchain-done out/wchain.log; do sleep 10; done
./watch.sh wA15 "seeds=6;tune=opp.hk_downed_leave_frames=15|mate.hk_downed_leave_frames=15"
./watch.sh wA6 "seeds=6;tune=opp.hk_downed_leave_frames=6|mate.hk_downed_leave_frames=6"
./watch.sh wB "seeds=6;tune=opp.hk_long_pass_m=30|mate.hk_long_pass_m=30"
./watch.sh wC "seeds=6;tune=opp.hk_own_zone_turn=1|mate.hk_own_zone_turn=1"
./watch.sh wD "seeds=6;tune=opp.hk_clump_frames=45|mate.hk_clump_frames=45"
./watch.sh wA6s "seeds=6;tune=opp.hk_downed_leave_frames=6|mate.hk_downed_leave_frames=6|opp.hk_avoid_downed_m=1.3|mate.hk_avoid_downed_m=1.3|opp.hk_avoid_downed_out=0.7|mate.hk_avoid_downed_out=0.7"
echo wchain2-done
