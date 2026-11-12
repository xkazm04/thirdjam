#!/usr/bin/env bash
# build.sh <name>: retarget, verify on the re-imported main file, verify the loopcut file
S=C:/Users/kazda/jam/skater-lab/skh; B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"; k=$1
"$B" -b --factory-startup --python $S/retarget.py -- $S $S/cfg/$k.json > $S/logs/retarget_$k.log 2>&1
"$B" -b --factory-startup --python $S/verify.py -- $S/export/$k/SK_Skater_$k.fbx $S/out/$k/verify.json $S/out/$k/retarget_report.json > $S/logs/verify_$k.log 2>&1 &
"$B" -b --factory-startup --python $S/verify.py -- $S/export/$k/SK_Skater_${k}_loopcut.fbx $S/out/$k/verify_loopcut.json $S/out/$k/retarget_report.json > $S/logs/verify_lc_$k.log 2>&1 &
wait
