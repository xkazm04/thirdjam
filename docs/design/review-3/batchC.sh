#!/usr/bin/env bash
# Lane A169 (REVIEW-3), batch C: the twelve modes on REVIEW-2's seeds (the shipped press, as MODES-6), then five rows of MODES-6 on fresh seeds.
R=docs/design/review-3/raw
node tools/playtest/modes.mjs --seeds 1-40 --jobs 3 --out $R/modes-regular > $R/modes-regular.out 2>&1
node tools/playtest/modes.mjs --seeds 1-40 --jobs 3 --player novice --out $R/modes-novice > $R/modes-novice.out 2>&1
node tools/playtest/modes.mjs --modes fog-of-war,shotgun,outnumbered,marathon,sudden-death --seeds 1001-1200 --jobs 3 --out $R/modes-fresh > $R/modes-fresh.out 2>&1
node tools/playtest/modes.mjs --modes fog-of-war,shotgun,outnumbered,sniper --seeds 1201-1300 --player novice --jobs 3 --out $R/modes-fresh-novice > $R/modes-fresh-novice.out 2>&1
node tools/playtest/modes.mjs --modes fog-of-war,cage,power-play --seeds 1301-1400 --player good --jobs 3 --out $R/modes-fresh-good > $R/modes-fresh-good.out 2>&1
echo done > $R/batchC.done
