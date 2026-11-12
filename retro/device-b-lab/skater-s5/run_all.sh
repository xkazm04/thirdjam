#!/usr/bin/env bash
# S5 hybrid, everything in one command (about 4 min): bash C:/Users/kazda/jam/skater-lab/s5/run_all.sh
# Inputs are the snapshots in s5/in/: SK_Skater_S4.fbx (S4 export 12:09, cm), R_rigged.fbx (s3/RIG2), strip_tracks.py (S4's).
# To pick up a newer S4: copy C:/Users/kazda/jam/skater-lab/s4/export/SK_Skater_S4.fbx into s5/in/ first.
set -e
B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"
S=C:/Users/kazda/jam/skater-lab/s5
E=$S/export
"$B" -b --factory-startup --python $S/retarget.py -- $S 2>&1 | grep -E "^S5|Error|Traceback" | tee $S/retarget.log
for f in SK_Skater_S5 SK_Skater_S5_loopcut; do
  : # DISABLED 12:35 by the lead (same reason as s4):   "$B" -b --factory-startup --python $S/in/strip_tracks.py -- $E/$f.fbx root Armature 2>&1 | grep -E "^STRIP|Error|Traceback"
done
: > $S/fbx_nodes.txt
for f in SK_Skater_S5 SK_Skater_S5_loopcut; do
  echo "== $f" >> $S/fbx_nodes.txt
  "$B" -b --factory-startup --python $S/fbx_nodes_s5.py -- $E/$f.fbx 2>&1 | grep -E "^(GLOBAL|MODEL)" >> $S/fbx_nodes.txt
  "$B" -b --factory-startup --python $S/in/fbx_tracks.py -- $E/$f.fbx 2>&1 | grep "^TRACKS" >> $S/fbx_nodes.txt || true
done
"$B" -b --factory-startup --python $S/verify.py -- $E/SK_Skater_S5.fbx $S/verify.json 2>&1 | grep -E "^VERIFY|Error|Traceback" | cut -c1-400
"$B" -b --factory-startup --python $S/verify.py -- $E/SK_Skater_S5_loopcut.fbx $S/verify_loopcut.json 2>&1 | grep -E "^VERIFY|Error|Traceback" | cut -c1-400
"$B" -b --factory-startup --python C:/Users/kazda/jam/thirdjam/tools/meshy/inspect.py -- $E/SK_Skater_S5.fbx $E/SK_Skater_S5_loopcut.fbx 2>&1 | grep '^{"tool"' > $S/inspect.jsonl
bash $S/render_all.sh
