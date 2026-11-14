#!/usr/bin/env bash
# Lane A169 (REVIEW-3): REVIEW-2's three traced cells, seeds 1-12, the ladder's instrument. Three processes side by side. Run from the worktree's root.
R=docs/design/review-3/raw
UE="C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
P="$(pwd -W)/Game/ThirdJam.uproject"
[ -f "$P" ] || P="$(ls "$(pwd -W)"/Game/*.uproject | head -1)"
export JAM_PLAYTESTER_CHECK=closing
run() { "$UE" "$P" -run=JamTournament -sim=jam -cartridge=hockey.regular -difficulty=$1 -player=$2 -seeds=1-12 -maxframes=36000 -trace=1 -nullrhi -Multiprocess -unattended -nopause -nosplash -stdout -FullStdOutLogOutput > $R/$3.log 2>&1; }
run rookie novice rk-novice &
run pro regular pro-regular &
run all_star good as-good &
wait
for n in rk-novice pro-regular as-good; do
  node docs/design/review-2/laws.mjs $R/$n.log > docs/design/review-3/data/$n.laws.json 2> $R/$n.laws.err
  node docs/design/review-1/humancarry.mjs $R/$n.log > docs/design/review-3/data/$n.carry.json 2> $R/$n.carry.err
done
echo done > $R/traces.done
