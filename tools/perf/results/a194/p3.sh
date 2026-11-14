#!/bin/sh
# lane A194 chain 3: CarryTrace at pro (100 matches): breakaway 0.2 / 0.3; then the stop-line instrument (STOPMARK=brake) with far 8.5 / 9 / 9.5
cd "$(dirname "$0")"
echo "p3 $$" >> out/pids.txt
sh trace.sh tr-ba02 "level=pro;player=regular;seeds=100;set=skater.breakaway.speed_bonus:0.2"
sh trace.sh tr-ba03 "level=pro;player=regular;seeds=100;set=skater.breakaway.speed_bonus:0.3"
for f in 8.5 9 9.5; do JAM_PLAYTESTER_STOPMARK=brake JAM_MARK_RULE="far=$f" sh trace.sh tr-obey-f$f "level=pro;player=regular;seeds=100"; done
echo p3-done
