#!/usr/bin/env bash
# Contact sheets for S5, all rendered from the exported (re-imported) FBX. Usage: bash render_all.sh
set -e
B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"
S=C:/Users/kazda/jam/skater-lab/s5
F=$S/export/SK_Skater_S5.fbx
S4F=$S/in/SK_Skater_S4.fbx
RF=$S/in/R_rigged.fbx
S2=C:/Users/kazda/jam/skater-lab/s2/engine/R
R=$S/r; SH=$S/sheets
mkdir -p $R $SH

# row <name> <file> <mode> <action|-> [extra args...]  -> $R/<name>.png (8 frames in one row)
row() {
  local name=$1 file=$2 mode=$3 act=$4; shift 4
  local aa=(); [ "$act" != "-" ] && aa=(--action "$act")
  "$B" -b --factory-startup --python $S/render_s5.py -- --in "$file" --out $R/$name --mode $mode --zoom 0.85 --tz 0.72 "${aa[@]}" "$@" > $R/$name.log 2>&1
  local n=$(ls $R/$name/f*.png | wc -l)
  ffmpeg -y -loglevel error -i $R/$name/f%02d.png -vf "tile=${n}x1" $R/$name.png
}
stack() {   # stack <out> <rows...>
  local out=$1; shift; local ins=() ; for r in "$@"; do ins+=(-i $R/$r.png); done
  ffmpeg -y -loglevel error "${ins[@]}" -filter_complex "vstack=inputs=$#,split[a][b];[a]palettegen=max_colors=192[p];[b][p]paletteuse=dither=none" $SH/$out.png
}

row tt     $F turntable -           --n 8
row tt_g   $F turntable Glide       --n 8
row st_34  $F front34   Stride
row st_sd  $F side      Stride
row st_bk  $F back      Stride
row gl_34  $F front34   Glide
row fo_34  $F front34   Idle_Faceoff
row sl_sd  $F side      Shot_Slap
row sl_34  $F front34   Shot_Slap
row wr_sd  $F side      Shot_Wrist
row ck_34  $F front34   Check
row sp_34  $F front34   Stop
row fa_sd  $F side      Fall
row gu_sd  $F side      GetUp
row ce_34  $F front34   Celebrate
row ce_fr  $F front     Celebrate
row ce_sd  $F side      Celebrate
row g_st   $F game      Stride     --size 96
row g_sl   $F game      Shot_Slap  --size 96
row g_fa   $F game      Fall       --size 96
row g_gl   $F game      Glide      --size 96
row cl_bk  $F back      Crossover_Left
row cr_34  $F front34   Crossover_Right
row ff_sd  $F side      Fall_Forward
row gf_ff  $F side      Fall_Forward
row gf_sd  $F side      GetUp_Front
row gf_34  $F front34   GetUp_Front
row ll_bk  $F back      Lean_Left  --n 4
row lr_bk  $F back      Lean_Right --n 4
row ll_fr  $F front     Lean_Left  --n 4
row lr_fr  $F front     Lean_Right --n 4
# comparison at the same camera: S4, S5, R + S2 text-to-motion (R turned to face +X)
row c_s4_st $S4F side Stride
row c_s5_st $F   side Stride
row c_r2_st $RF  side - --anim-fbx $S2/Stride.fbx --rotz 90
row c_s4_sl $S4F side Shot_Slap
row c_s5_sl $F   side Shot_Slap
row c_r2_sl $RF  side - --anim-fbx $S2/Shot_Slap.fbx --rotz 90

stack turntable tt tt_g
stack skating st_34 st_sd st_bk gl_34 fo_34
stack shots sl_sd sl_34 wr_sd ck_34 sp_34
stack fall fa_sd gu_sd ce_34 ce_fr ce_sd
ffmpeg -y -loglevel error -i $R/g_st.png -i $R/g_sl.png -i $R/g_gl.png -i $R/g_fa.png -filter_complex "vstack=inputs=4,scale=iw*2:ih*2:flags=neighbor" $SH/distance.png
stack extra cl_bk cr_34 ff_sd
stack getup_front gf_ff gf_sd gf_34
ffmpeg -y -loglevel error -i $R/ll_bk.png -i $R/lr_bk.png -i $R/ll_fr.png -i $R/lr_fr.png -filter_complex "[0][1]hstack[a];[2][3]hstack[b];[a][b]vstack" $SH/leans.png
stack compare c_s4_st c_s5_st c_r2_st c_s4_sl c_s5_sl c_r2_sl
ls -la $SH
