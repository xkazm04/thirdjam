#!/bin/sh
# lane A208: READY 1's measurements on the final build: (a) the possession tests then the full gate, one editor; (b) the six cells x 600, then the neutral proof N3, two workers; (c) after (a): law 3 at the pickup's default and neutral.
cd "$(dirname "$0")"
echo "chain-final3 $$" >> out/pids.txt
( sh jam.sh poss7 "Jam.Core.Possession+Jam.Controls.FirstFiveMinutes" > out/poss7.out 2>&1; sh jam.sh gate21 Jam > out/gate21.out 2>&1; JAM_LAW3_AT="8,9.5" sh law3.sh law3a21 "pro:-|all_star:-" ; JAM_LAW3_SET="pickup.arc_m=0;pickup.body=0" JAM_LAW3_AT="8,9.5" sh law3.sh law3n21 "pro:-|all_star:-"; echo a-done > out/final3.a ) &
( W=2 sh cells.sh R5 600 "" > out/chainR5.log 2>&1; rm -f raw/N3-*.jsonl; sh proof.sh N3 > out/proofN3.log 2>&1; echo b-done > out/final3.b ) &
wait
echo final-done > out/final3.done
