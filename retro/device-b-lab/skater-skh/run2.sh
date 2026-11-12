#!/usr/bin/env bash
# SKH second pass (about 4 min): retarget from the SKF scripted family into export2/, verify, bind check against export/, table, sheets.
# Inputs: in/skf/SK_Skater_{p2_cel,p3_clay,p4_broadcast}.fbx (copies of skf/export at 15:33), cfg/2_<name>.json.
# Never writes into export/ (another lane imports from it). Track strip stays disabled.
set -e
S=C:/Users/kazda/jam/skater-lab/skh
B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"
for k in P4B P4B_ctl_p2 P3A P3A_ctl_p2 R; do bash $S/build2.sh $k & done; wait
for k in P4B P3A R; do "$B" -b --factory-startup --python $S/bindcheck.py -- $S/export/$k/SK_Skater_$k.fbx $S/export2/$k/SK_Skater_$k.fbx $S/out2/$k/bindcheck.json 2>&1 | grep "^BIND" | cut -c1-200; done
"$B" -b --factory-startup --python $S/bindcheck.py -- $S/export/P4B/SK_Skater_P4B.fbx $S/export2/P4B_ctl_p2/SK_Skater_P4B.fbx $S/out2/P4B_ctl_p2/bindcheck.json 2>&1 | grep "^BIND" | cut -c1-200
for k in P4B P3A R; do "$B" -b --factory-startup --python $S/verify.py -- $S/export/$k/SK_Skater_$k.fbx $S/out/$k/verify_v2.json $S/out/$k/retarget_report.json > /dev/null 2>&1 & done; wait
node $S/old_new.mjs > $S/old_new.md
bash $S/render2.sh R R; bash $S/render2.sh P3A P3A; bash $S/render2.sh P4B P4B; bash $S/render2.sh P4B_ctl_p2 P4B
