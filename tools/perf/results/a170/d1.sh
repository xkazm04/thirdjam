#!/bin/sh
# lane A170 item 1: pro's hunt radius as a dial with the presser kept on its job (commit 8 m), hockey.regular, closing press (the ladder's instrument).
# law 3 over every arm (one process) beside the cells (two workers): three processes.
cd "$(dirname "$0")"
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
sh law3.sh law3-1 "pro:-|pro:$K;opp.hunt_radius_m=8|pro:$K;opp.hunt_radius_m=10|pro:$K;opp.hunt_radius_m=11|pro:$K;opp.hunt_radius_m=12|pro:$K;opp.hunt_radius_m=13|pro:$K;opp.hunt_radius_m=14|pro:$K;opp.hunt_radius_m=16" &
node curve.mjs D-pro-reg-k8-n100 regular pro regular 100 "$K;opp.hunt_radius_m=8" --workers 2 --check closing > /dev/null
node same.mjs D-pro-reg-k8-n100 A162-B-pro-reg
for h in 12 10 14 11 13; do
  node curve.mjs D-pro-reg-h$h regular pro regular 600 "$K;opp.hunt_radius_m=$h" --workers 2 --check closing > /dev/null
done
wait
cat ../../../../Game/logs/a170/law3-1.txt
