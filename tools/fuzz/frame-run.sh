#!/bin/sh
# Lane A198 item 6: the frame on clay and on the neon tier, five a side, the boosts' beacons standing (the boost cut's pickups) and goals on set frames
# (the GOAL banner), off screen, one package after the other (never two games at once: the GPU is shared). Arms: "before" = a copy of the 13:21 Accept_H32
# package (no beacons, no banner), "after" = this tree's Development package. Usage: sh tools/fuzz/frame-run.sh <before exe> <after exe> [rounds]
cd "$(dirname "$0")/../.."
B=$1; A=$2; N=${3:-2}
for r in $(seq 1 $N); do
  for tier in p3_clay l3_neon; do
    for arm in before after; do
      exe=$B; [ $arm = after ] && exe=$A
      powershell -NoProfile -ExecutionPolicy Bypass -File tools/fuzz/frame-bot.ps1 -Exe "$exe" -Tier $tier -Name ${TAG:-a198}-$arm-$tier-$r -Offscreen -PlaySeconds 100 -Port ${PORT:-4812} > tools/fuzz/out/frame-$arm-$tier-$r.txt 2>&1
      echo "$(date +%H:%M:%S) $arm $tier r$r exit $?"
    done
  done
done
for d in tools/fuzz/out/frame/${TAG:-a198}-*; do echo "== $d"; node tools/perf/five.mjs "$d" 2>&1 | head -6; done
for t in p3_clay l3_neon; do echo "== goals $t"; node tools/perf/goalpair.mjs tools/fuzz/out/frame/${TAG:-a198}-*-$t-* --base ${TAG:-a198}-before-$t 2>&1 | tail -15; done
