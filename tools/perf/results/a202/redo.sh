#!/bin/sh
# lane A202: FA-pro-large again (its first run crashed in the driver's worker race), after famchain, one worker
cd "$(dirname "$0")"
echo "chain-redo $$" >> out/pids.txt
while ! grep -q fam-done out/fam.log; do sleep 15; done
node curve.mjs FA-pro-large regular pro large 600 "opp.hk_downed_leave_frames=6;mate.hk_downed_leave_frames=6;opp.hk_avoid_downed_m=1.3;mate.hk_avoid_downed_m=1.3;opp.hk_avoid_downed_out=0.7;mate.hk_avoid_downed_out=0.7" --workers 1 --check closing > /dev/null
echo redo-done
