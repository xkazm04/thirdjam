#!/usr/bin/env bash
# build2.sh <name>: second pass (cfg/2_<name>.json): retarget, verify main and loopcut into out2/<name>/
S=C:/Users/kazda/jam/skater-lab/skh; B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"; k=$1
a=$(grep -o '"asset": "[^"]*"' $S/cfg/2_$k.json | cut -d'"' -f4)
"$B" -b --factory-startup --python $S/retarget.py -- $S $S/cfg/2_$k.json > $S/logs/r2_$k.log 2>&1
"$B" -b --factory-startup --python $S/verify.py -- $S/export2/$k/SK_Skater_$a.fbx $S/out2/$k/verify.json $S/out2/$k/retarget_report.json > $S/logs/v2_$k.log 2>&1 &
"$B" -b --factory-startup --python $S/verify.py -- $S/export2/$k/SK_Skater_${a}_loopcut.fbx $S/out2/$k/verify_loopcut.json $S/out2/$k/retarget_report.json > $S/logs/v2lc_$k.log 2>&1 &
wait
