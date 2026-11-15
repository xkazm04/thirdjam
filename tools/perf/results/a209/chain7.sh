#!/bin/sh
# lane A209 wave 3 final (m6: + the brace fault's fix): tests before / after, neutral vs M1N, the person hand, law 3, the fix's reach, the gate
cd "$(dirname "$0")"
R=../../../..
echo "$(date +%H:%M:%S) chain7 sh $$" >> out/pids.txt
( cd $R && bash tools/perf/results/a209/ue.sh "Jam.Core.Contact.TapNeverBraces" tap-before JAM_TEST_A151=hold_reset=0 ) > out/tap-before.txt &
( cd $R && bash tools/perf/results/a209/ue.sh "Jam.Core.Contact" contact-after ) > out/contact-after.txt
wait
EXTRA="--seam check_reach=0;check_pull=0;cross_any=0;check_windup=4;cross_windup=4;hold_reset=0" sh hashes.sh F-N
for c in rk pro as; do for r in reg large; do node same.mjs M1N-$c-$r F-N-$c-$r; done; done > out/F-neutral.txt
for lv in rk pro; do case $lv in rk) c=novice; l=rookie;; pro) c=regular; l=pro;; esac
  node curve.mjs F-person-$lv $c $l regular 100 "" --workers 2 --check closing --hand person > /dev/null; done
node hand-table.mjs F-person-rk F-person-pro > out/F-person.json
( EXTRA_ENV="JAM_LAW3_AT=8,8.5,9,9.5" sh law3.sh F-law3 "pro:-|all_star:-" ) &
# the fix's reach: the shipped default match with and without it, 100 a cell (C3's six cells at 600 were played without it)
sh hashes.sh F-S
EXTRA="--seam hold_reset=0" sh hashes.sh F-S0
for c in rk pro as; do for r in reg large; do node same.mjs F-S-$c-$r F-S0-$c-$r; done; done > out/F-fixreach.txt
wait
( cd $R && bash tools/perf/results/a209/ue.sh "Jam" gate5 && grep -E "Result=\{Fail" Game/logs/a209/gate5.log | sed 's/.*Path={\([^}]*\)}.*/\1/' ) > out/gate5.txt
echo "chain7 done $(date +%H:%M:%S)"
