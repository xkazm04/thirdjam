#!/usr/bin/env bash
# SKH, everything that needs no credits (about 6 min): bash C:/Users/kazda/jam/skater-lab/skh/run_all.sh
# Inputs: in/SK_Skater_S4.fbx (S4's 14:09 export, 31 takes, copied from s5/in), in/<cand>/<cand>_rigged.fbx, cfg/<cand>.json,
# and the base-colour / normal PNGs in export/<cand>/ (made by tex/: extract_detect.py, clean_gB.py, clay_r.py).
# The track-strip step stays DISABLED (as the lead set it in s5 at 12:35); no step here strips tracks.
set -e
S=C:/Users/kazda/jam/skater-lab/skh
B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"
mkdir -p $S/logs
# 1. retarget + verify (main and loopcut) for every figure; R is the S5 regression build, Rm the measured-only R (diagnostic)
for k in R Rm Rclay gB P4A P4B P3A P3B; do bash $S/build.sh $k & done; wait
# 2. regression: R against s5/export, frame by frame, all 31 takes, both files
"$B" -b --factory-startup --python $S/compare.py -- C:/Users/kazda/jam/skater-lab/s5/export/SK_Skater_S5.fbx $S/export/R/SK_Skater_R.fbx $S/out/R/compare_s5.json 2>&1 | grep "^COMPARE"
"$B" -b --factory-startup --python $S/compare.py -- C:/Users/kazda/jam/skater-lab/s5/export/SK_Skater_S5_loopcut.fbx $S/export/R/SK_Skater_R_loopcut.fbx $S/out/R/compare_s5_loopcut.json 2>&1 | grep "^COMPARE"
node $S/summary.mjs R gB P4A P4B P3A P3B Rclay Rm > $S/summary.md
# 3. sheets (Workbench); Rclay and the P3 comparison use tex/render_clay.py (EEVEE), see the status report for those commands
for k in gB P4A P4B P3A P3B; do bash $S/render_cand.sh $k; done
