#!/usr/bin/env bash
# Lane A169 (REVIEW-3), batch D: spot checks and three measured "smallest changes" (variants written by the driver under tools/playtest/out/, none shipped).
# 200 seeds a cell, three processes at a time.
R=docs/design/review-3/raw
cell() { node tools/playtest/run.mjs --players $1 --levels $2 --rinks $3 --seeds 1-200 --jobs 1 --out $R/$4 "${@:5}" > $R/$4.out 2>&1; }
# the masher of CONTACT-7 on this commit (the shipped press, as that lane measured; the base is grid200-shipped)
( export JAM_PLAYTESTER_MASH=12; cell novice rookie regular mash-rk & cell regular pro regular mash-pro & cell good all_star regular mash-as & wait )
# the long shot at rookie alone, and big ice with a carrier that can be caught (the ladder's instrument; the bases are grid200 and large200-base)
( export JAM_PLAYTESTER_CHECK=closing
  cell novice rookie regular shoot13-rk --tune "opp.shoot_extra_m=13" & cell regular pro large large200-base & cell regular pro large large200-carry08 --set "skater.carry_speed_mult=0.8" & wait
  cell novice rookie large large200-rk-base & cell novice rookie large large200-rk-carry08 --set "skater.carry_speed_mult=0.8" & wait )
# Cage against the regular game at 200 seeds (its third on this commit)
node tools/playtest/modes.mjs --modes cage,bruisers --seeds 1-200 --jobs 3 --out $R/modes-cage200 > $R/modes-cage200.out 2>&1
echo done > $R/batchD.done
