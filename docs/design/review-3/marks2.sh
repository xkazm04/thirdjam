#!/usr/bin/env bash
# Lane A169 (REVIEW-3): one set of numbers for every level, on H29's far edge (9.5 m): the contested ring's closing speed and the stop mark's, 100 matches an arm.
D=docs/design/review-3
arm() { JAM_CARRY_TRACE="level=$1;player=$2;seeds=100" JAM_MARK_RULE="$3" bash $D/auto.sh "mark-$4" "Bar.Movement.CarryTrace"; }
arm rookie novice "closing=14;stopclosing=10;far=9.5" rk-c14s10far95 & arm pro regular "closing=14;stopclosing=10;far=9.5" pro-c14s10far95 & arm all_star good "closing=14;stopclosing=10;far=9.5" as-c14s10far95 & wait
arm pro regular "closing=12;stopclosing=8;far=9.5" pro-c12s8far95 & arm all_star good "closing=12;stopclosing=8;far=9.5" as-c12s8far95 & wait
echo done > $D/raw/marks2.done
