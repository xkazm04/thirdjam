#!/usr/bin/env bash
# Lane A200: Bar.Curve.Law3 at each bots' wind-up (both keys through the JAM_TEST_A151 seam), the brain's brake switch off and on in one process
# (JAM_LAW3_TUNES: four arms), the gate's rows and the late ones (JAM_LAW3_AT). Three processes at a time (ue.sh records each editor pid).
#   bash law3.sh ship 4 6 10        ship = the cartridge as it is (bots 8 / 12 since build3); N = both bots' keys at N (4 = as main)
cd "$(dirname "$0")/../../../.."
T='pro:-|all_star:-|pro:opp.hk_windup_brake=1|all_star:opp.hk_windup_brake=1'
AT='8,8.5,9,9.5,10,11,12'
n=0
for w in "$@"; do
  if [ "$w" = ship ]; then seam=''; else seam="bot_check_windup=$w;bot_cross_windup=$w"; fi
  bash tools/perf/results/a200/ue.sh "Bar.Curve.Law3" "law3w$w" "JAM_LAW3_TUNES=$T" "JAM_LAW3_AT=$AT" ${seam:+"JAM_TEST_A151=$seam"} &
  n=$((n+1)); if [ $((n % 3)) = 0 ]; then wait; fi
done
wait
echo "law3 done $(date +%H:%M:%S)"
