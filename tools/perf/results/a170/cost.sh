#!/bin/sh
# lane A170 DIAL COST: REVIEW-3's traced pro cell (docs/design/review-3/traces.sh: -trace=1, closing press) at hunt 8 / 11 / 12, seeds 1-40, three processes;
# read by review-2/laws.mjs and review-1/humancarry.mjs (the main checkout's copies, read only)
cd "$(dirname "$0")/../../../.."
R=Game/logs/a170; M=/c/Users/kazda/kiro/thirdjam/docs/design
UE="C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"; P="$(pwd -W)/Game/ThirdJam.uproject"
export JAM_PLAYTESTER_CHECK=closing
K="opp.hk_press_in_hunt=1;opp.hk_press_commit_m=8"
run() { "$UE" "$P" -run=JamTournament -sim=jam -cartridge=hockey.regular -difficulty=pro -player=regular -seeds=1-40 -maxframes=36000 -trace=1 $2 -nullrhi -Multiprocess -unattended -nopause -nosplash -stdout -FullStdOutLogOutput > $R/$1.log 2>&1; }
run tr-h8 "" &
run tr-h11 "-aitune=$K;opp.hunt_radius_m=11" &
run tr-h12 "-aitune=$K;opp.hunt_radius_m=12" &
wait
for n in tr-h8 tr-h11 tr-h12; do
  node $M/review-2/laws.mjs $R/$n.log > $R/$n.laws.json 2> $R/$n.laws.err
  node $M/review-1/humancarry.mjs $R/$n.log > $R/$n.carry.json 2> $R/$n.carry.err
done
echo cost-done
