#!/bin/sh
# usage: pair.sh <tag> [extra args]   large + regular at pro, the regular class, seeds 1-100, traced, two processes side by side; then the tables.
# The environment (JAM_BRAIN2_OFF=letters) passes through.
W=/c/Users/kazda/kiro/thirdjam/.claude/worktrees/a105
D=$W/Game/logs/a115
S=$W/tools/perf/results/a115
TAG=$1; shift
$S/pt.sh $TAG-large-pro hockey.arena.large pro regular 1-100 30 "$@" &
$S/pt.sh $TAG-regular-pro hockey.regular pro regular 1-100 30 "$@" &
wait
node $S/agg.mjs $D/$TAG-large-pro.log $D/$TAG-regular-pro.log
for t in large regular; do echo "== $t"; node $W/tools/soak/playtest/idle.mjs $D/$TAG-$t-pro.log | grep -E "30 s:|opp_carry|mate_carry|loose:|p50"; done
