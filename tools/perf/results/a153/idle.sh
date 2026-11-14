#!/bin/bash
# Lane A153 (lane A134's idle.sh; DRIVER=tools/perf/results/a153/cell.mjs takes --extra). usage: idle.sh <label> <level> <regular|large> <n> [tune] [extra curve.mjs options]
# One curve cell (the regular class) through tools/perf/results/a95/curve.mjs with --workers 3, then one line: the share of matches whose longest stretch
# without a human touch is 30 s or more, its p50 / p90, the win rate and what the human's side did. Raw matches: tools/perf/results/a95/raw/<label>.jsonl.
cd "$(dirname "$0")/../../../.."
L=$1; LVL=$2; RINK=$3; N=$4; TUNE=$5; shift; shift; shift; shift; shift
CLS=${CLS:-regular}
node ${DRIVER:-tools/perf/results/a95/curve.mjs} $L $CLS $LVL $RINK $N "$TUNE" --workers ${WORKERS:-3} "$@" > /dev/null 2>Game/logs/a153/$L.err
node tools/perf/results/a134/sum.mjs $L
