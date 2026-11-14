#!/usr/bin/env bash
# Lane A169 (REVIEW-3), batch A: REVIEW-2's three commands on the same seeds, three processes at a time, the ladder's instrument
# (JAM_PLAYTESTER_CHECK=closing: the press the ladder was set with and REVIEW-2 was measured with), then the grid again with the shipped press.
# Run from the worktree's root. Raw output: docs/design/review-3/raw/ (does not travel).
R=docs/design/review-3/raw
export JAM_PLAYTESTER_CHECK=closing
node tools/playtest/run.mjs --players novice,regular,good --levels rookie,pro,all_star --rinks regular --seeds 1-40 --jobs 3 --out $R/grid > $R/grid.out 2>&1
node tools/playtest/run.mjs --players novice,regular,good --levels rookie --rinks ladder/take-1,ladder/take-2,ladder/take-3,ladder/take-4 --seeds 1-60 --jobs 3 --out $R/ladder1234 > $R/ladder1234.out 2>&1
node tools/playtest/run.mjs --players novice,regular,good --levels pro --rinks ladder/take-5,ladder/take-6,ladder/take-7 --seeds 1-60 --jobs 3 --out $R/ladder567 > $R/ladder567.out 2>&1
node tools/playtest/run.mjs --players novice,regular,good --levels all_star --rinks ladder/take-8 --seeds 1-60 --jobs 3 --out $R/ladder8 > $R/ladder8.out 2>&1
node tools/playtest/run.mjs --players novice,regular,good --levels rookie,pro,all_star --rinks large --seeds 1-40 --jobs 3 --out $R/large > $R/large.out 2>&1
node tools/playtest/run.mjs --players novice,regular,good --levels rookie,pro,all_star --rinks regular --seeds 1-200 --jobs 3 --out $R/grid200 > $R/grid200.out 2>&1
unset JAM_PLAYTESTER_CHECK
node tools/playtest/run.mjs --players novice,regular,good --levels rookie,pro,all_star --rinks regular --seeds 1-200 --jobs 3 --out $R/grid200-shipped > $R/grid200-shipped.out 2>&1
echo done > $R/batchA.done
