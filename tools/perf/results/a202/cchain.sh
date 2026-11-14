#!/bin/sh
# lane A202: on build C (the final tree): each fix alone, six cells x 600 (FA-*, FC-*, FD-*, FB-*), then the hash proofs, 100 seeds a cell:
# N8-* brain8=0 and D8-* brain8 at its default against main's B-* (build A). Child pids in out/pids.txt.
cd "$(dirname "$0")"
echo "chain-c $$" >> out/pids.txt
A="opp.hk_downed_leave_frames=6;mate.hk_downed_leave_frames=6;opp.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_m=1.3;opp.hk_avoid_downed_out=0.7;mate.hk_avoid_downed_out=0.7"
W=2 ./cells.sh FA 600 "$A"
W=2 ./cells.sh FC 600 "opp.hk_own_zone_turn=1;mate.hk_own_zone_turn=1"
W=2 ./cells.sh FD 600 "opp.hk_clump_frames=45;mate.hk_clump_frames=45"
W=2 ./cells.sh FB 600 "opp.hk_long_pass_m=30;mate.hk_long_pass_m=30"
echo cells-done
W=2 ./cells.sh N8 100 "brain8=0"
W=2 ./cells.sh D8 100 ""
for c in pro-reg as-reg rk-reg pro-large as-large rk-large; do node same.mjs B-$c N8-$c; node same.mjs B-$c D8-$c; done
echo cchain-done
