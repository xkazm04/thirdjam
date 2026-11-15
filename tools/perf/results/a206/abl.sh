#!/bin/sh
# lane A206: what makes all_star large easier on m5 (good player, 200)
cd "$(dirname "$0")"
echo "abl $$" >> out/pids.txt
run() { node curve.mjs "$1" good all_star large 200 "$2" --workers 1 --check closing > /dev/null; }
run AB-dodge0 "opp.hk_dodge9_share=0" &
run AB-lane0 "opp.hk_shot_lane9=0;mate.hk_shot_lane9=0" &
run AB-drive0 "opp.hk_drive_net9_m=0;mate.hk_drive_net9_m=0" &
wait
echo abl-done >> out/pids.txt
