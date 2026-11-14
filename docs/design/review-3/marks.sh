#!/usr/bin/env bash
# Lane A169 (REVIEW-3): the two marks' rules swept without a build (JAM_MARK_RULE, lane A166's switch), three processes at a time, 100 matches an arm.
D=docs/design/review-3
arm() { JAM_CARRY_TRACE="level=$1;player=$2;seeds=100" JAM_MARK_RULE="$3" bash $D/auto.sh "mark-$4" "Bar.Movement.CarryTrace"; }
arm rookie novice "far=9.5" rk-far95 & arm pro regular "far=9.5" pro-far95 & arm all_star good "far=9.5" as-far95 & wait
arm rookie novice "closing=12;stopclosing=8" rk-c12s8 & arm rookie novice "closing=14;stopclosing=10" rk-c14s10 & arm rookie novice "closing=12;stopclosing=8;far=9.5" rk-c12s8far95 & wait
echo done > $D/raw/marks.done
