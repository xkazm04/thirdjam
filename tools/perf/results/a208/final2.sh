#!/bin/sh
# lane A208: READY 1's measurements on the final build: (a) the possession tests then the full gate, one editor; (b) the six cells x 600, then the neutral proof N2, two workers; (c) after (a): law 3 at the pickup's default and neutral.
cd "$(dirname "$0")"
echo "chain-final2 $$" >> out/pids.txt
( sh jam.sh poss6 "Jam.Core.Possession+Jam.Controls.FirstFiveMinutes" > out/poss6.out 2>&1; sh jam.sh gate15 Jam > out/gate15.out 2>&1; JAM_LAW3_AT="8,9.5" sh law3.sh law3a15 "pro:-|all_star:-" ; JAM_LAW3_SET="pickup.arc_m=0;pickup.body=0" JAM_LAW3_AT="8,9.5" sh law3.sh law3n15 "pro:-|all_star:-"; echo a-done > out/final2.a ) &
( W=2 sh cells.sh R4 600 "" > out/chainR4.log 2>&1; rm -f raw/N2-*.jsonl; sh proof.sh N2 > out/proofN2.log 2>&1; echo b-done > out/final2.b ) &
wait
echo final-done > out/final2.done
