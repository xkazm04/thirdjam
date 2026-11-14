#!/bin/sh
# lane A208: READY 1's measurements on the final build: (a) the possession tests then the full gate, one editor; (b) the six cells x 600, then the neutral proof N1, two workers; (c) after (a): law 3 at the pickup's default and neutral.
cd "$(dirname "$0")"
echo "chain-final $$" >> out/pids.txt
( sh jam.sh poss5 "Jam.Core.Possession" > out/poss5.out 2>&1; sh jam.sh gate13 Jam > out/gate13.out 2>&1; JAM_LAW3_AT="8,9.5" sh law3.sh law3a "pro:-|all_star:-" ; JAM_LAW3_SET="pickup.arc_m=0;pickup.body=0" JAM_LAW3_AT="8,9.5" sh law3.sh law3n "pro:-|all_star:-"; echo a-done > out/final.a ) &
( W=2 sh cells.sh R3 600 "" > out/chainR3.log 2>&1; rm -f raw/N1-*.jsonl; sh proof.sh N1 > out/proofN1.log 2>&1; echo b-done > out/final.b ) &
wait
echo final-done > out/final.done
