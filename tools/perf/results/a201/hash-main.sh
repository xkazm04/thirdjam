#!/bin/sh
# lane A201: the default match (regular player, the ladder's closing press) on 100 seeds a level, on main 58af359f's binaries (the main checkout, read only) -> raw/main-<level>.jsonl
cd "$(dirname "$0")"
echo "hash-main $$" >> out/pids.txt
for l in rookie pro all_star; do JAM_UPROJECT='C:/Users/kazda/kiro/thirdjam/Game/ThirdJam.uproject' node curve.mjs main-$l regular $l regular 100 --check closing --workers 2; done
echo hash-main-done
