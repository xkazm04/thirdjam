#!/bin/sh
# lane A202: fix A on the human's mates only (FAm-*), one worker (the third measurement process)
cd "$(dirname "$0")"
echo "chain-fam $$" >> out/pids.txt
W=1 ./cells.sh FAm 600 "mate.hk_downed_leave_frames=6;mate.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_out=0.7"
echo fam-done
