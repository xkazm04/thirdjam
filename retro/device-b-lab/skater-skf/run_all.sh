#!/usr/bin/env bash
# SKF: one command rebuilds the scripted skater family (mesh + rig, 31 clips, exports, re-import checks, renders, sheets).
#   bash C:/Users/kazda/jam/skater-lab/skf/run_all.sh              all three styles in parallel, then the comparison sheet
#   bash C:/Users/kazda/jam/skater-lab/skf/run_all.sh p3_clay      one style (p2_cel | p3_clay | p4_broadcast)
# The FBX track-strip step of s4 stays disabled (Unreal 5.9 rejected the rewritten file); root keys are identity and harmless.
set -e
SUF="${SKF_SUFFIX:-}"; export SKF_SUFFIX="$SUF"   # SKF_SUFFIX=2: second pass into export2/ work2/ r2/ sheets2/
S="$(cd "$(dirname "$0")" && pwd -W 2>/dev/null || pwd)"
B="/c/Program Files/Blender Foundation/Blender 4.2/blender.exe"
bl() { "$B" -b --factory-startup --python "$@" 2>&1 | grep -vE "TBBmalloc|^$"; }
ff() { ffmpeg -y -loglevel error "$@"; }
PAL=",split[a][b];[a]palettegen=max_colors=128[p];[b][p]paletteuse=dither=none"

one() {
  st="$1"; L="$S/logs$SUF"; mkdir -p "$L"
  bl "$S/build_skater.py" -- "$st" "$S" > "$L/build_$st.log"
  grep -E "SKF BUILD OK|rror" "$L/build_$st.log" || true
  bl "$S/build_clips.py" -- "$st" "$S" > "$L/clips_$st.log"
  grep -E "SKF EXPORT OK|rror" "$L/clips_$st.log" || true
  E="$S/export$SUF/$st"; W="$S/work$SUF/$st"
  bl "$S/check_fbx.py" -- "$E/SK_Skater_$st.fbx" "$W/check_fbx.json" > "$L/check_$st.log"
  bl "$S/check_fbx.py" -- "$E/SK_Skater_${st}_loopcut.fbx" "$W/check_fbx_loopcut.json" > "$L/check_loopcut_$st.log"
  R="$S/r$SUF/$st"; rm -rf "$R"; mkdir -p "$R" "$S/sheets$SUF"
  bl "$S/render_skf.py" -- "$E/SK_Skater_$st.fbx" "$R" "$st" \
     turn_rest:turntable:-:8:200 turn:turntable:Glide:8:200 \
     m_stride:motion:Stride:8:160 s_stride:side:Stride:8:160 b_stride:back:Stride:8:160 s_slap:side:Shot_Slap:8:160 \
     g_blue:game:Stride:8:96:blue g_orange:game:Stride:8:96:orange g_slap:game:Shot_Slap:8:96:orange \
     cmp_glide:motion:Glide:1:240:blue:0 cmp_slap:side:Shot_Slap:1:240:blue:16 \
     s_fall:side:Fall:8:160 s_fallf:side:Fall_Forward:8:160 s_getup:side:GetUp:8:160 s_getupf:side:GetUp_Front:8:160 s_block:side:Goalie_Block_Left:8:160 \
     close:close:Glide:8:200 t_aimf:front:Aim_FarReach:1:200:blue:0 t_poke:side:Poke:1:200:blue:10 t_aim1:side:Aim_Front:1:200:blue:0 > "$L/render_$st.log"
  cd "$R"
  for j in s_fall s_fallf s_getup s_getupf s_block turn_rest turn close m_stride s_stride b_stride s_slap g_blue g_orange g_slap; do ff -i $j/f%02d.png -vf tile=8x1 row_$j.png; done
  ff -i row_turn_rest.png -i row_turn.png -i row_close.png -filter_complex "vstack=inputs=3$PAL" "$S/sheets$SUF/${st}_turntable.png"
  ff -i row_m_stride.png -i row_s_stride.png -i row_b_stride.png -i row_s_slap.png -filter_complex "vstack=inputs=4$PAL" "$S/sheets$SUF/${st}_stride_slap.png"
  ff -i row_g_blue.png -i row_g_orange.png -i row_g_slap.png -filter_complex "vstack=inputs=3,scale=iw*2:ih*2:flags=neighbor$PAL" "$S/sheets$SUF/${st}_distance_40px.png"
  ff -i row_s_fall.png -i row_s_fallf.png -i row_s_getup.png -i row_s_getupf.png -i row_s_block.png -filter_complex "vstack=inputs=5$PAL" "$S/sheets$SUF/${st}_falls_side.png"
  ff -i t_aim1/f00.png -i t_aimf/f00.png -i t_poke/f00.png -filter_complex "hstack=inputs=3$PAL" "$S/sheets$SUF/${st}_reach.png"
  cd "$S"
  echo "SKF DONE $st"
}

if [ -n "$1" ]; then
  one "$1"
else
  for st in p2_cel p3_clay p4_broadcast; do one "$st" & done
  wait
fi
if [ -d "$S/r$SUF/p2_cel/cmp_glide" ] && [ -d "$S/r$SUF/p3_clay/cmp_glide" ] && [ -d "$S/r$SUF/p4_broadcast/cmp_glide" ]; then
  cd "$S/r$SUF"
  ff -i p2_cel/cmp_glide/f00.png -i p3_clay/cmp_glide/f00.png -i p4_broadcast/cmp_glide/f00.png \
     -i p2_cel/cmp_slap/f00.png -i p3_clay/cmp_slap/f00.png -i p4_broadcast/cmp_slap/f00.png \
     -filter_complex "[0][1][2]hstack=inputs=3[t];[3][4][5]hstack=inputs=3[b];[t][b]vstack=inputs=2$PAL" "$S/sheets$SUF/compare_styles.png"
fi
ls -la "$S/sheets$SUF"
