#!/usr/bin/env bash
# Lane A200: what follows the table's chain, in order, three measurement processes at most at any time.
#   bash after.sh <pid of the chain shell to wait for>
cd "$(dirname "$0")/../../../.."
R=tools/perf/results/a200
while ps -p "$1" > /dev/null 2>&1; do sleep 10; done
echo "== after: start $(date +%H:%M:%S)"
E="JAM_PLAYTESTER_CHECK=closing"; B=contact.bot_check_windup_frames; X=contact.bot_cross_windup_frames
bash $R/run-arms.sh "c8x12 600 $B=8;$X=12 $E"
bash $R/run-arms-large.sh "L4 600 - $E" "Lc8x12 600 $B=8;$X=12 $E"
bash $R/law3.sh 4 8 12
bash $R/law3.sh 6 10
bash $R/ue.sh "Jam" gate0 &
bash $R/ue.sh "Jam" gate88 "JAM_TEST_A151=bot_check_windup=8;bot_cross_windup=8" &
wait
bash $R/run-arms.sh "finplain 100 -"
echo "== after: done $(date +%H:%M:%S)"
