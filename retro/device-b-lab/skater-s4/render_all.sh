#!/usr/bin/env bash
# Renders every proof image from the exported FBX and builds the contact sheets.
#   bash render_all.sh [fbx]       (default export/SK_Skater_S4.fbx)
set -e
S="$(cd "$(dirname "$0")" && pwd -W 2>/dev/null || pwd)"
B="/c/Program Files/Blender Foundation/Blender 4.2/blender.exe"
FBX="${1:-$S/export/SK_Skater_S4.fbx}"
R="$S/r"
mkdir -p "$R" "$S/sheets"
run() { "$B" -b --factory-startup --python "$S/render_s4.py" -- "$@" 2>&1 | grep -E "ACTION|DONE|Error|rror:" || true; }
CLIPS="Idle_Faceoff Stride Glide Stop Shot_Slap Shot_Wrist Check Fall Fall_Forward GetUp GetUp_Front Celebrate Crossover_Left Crossover_Right"
run "$FBX" "$R/turn" turntable Glide 8 200
run "$FBX" "$R/turn_rest" turntable - 8 200
for c in $CLIPS; do run "$FBX" "$R/m_$c" motion "$c" 8 160; done
run "$FBX" "$R/side_Stride" side Stride 8 160
run "$FBX" "$R/back_Stride" back Stride 8 160
run "$FBX" "$R/back_Crossover_Left" back Crossover_Left 8 160
run "$FBX" "$R/lean_L" turntable Lean_Left 4 160
run "$FBX" "$R/lean_R" turntable Lean_Right 4 160
run "$FBX" "$R/g_Stride_blue" game Stride 8 96 blue
run "$FBX" "$R/g_Stride_orange" game Stride 8 96 orange
run "$FBX" "$R/g_Shot_Slap_blue" game Shot_Slap 8 96 blue
run "$FBX" "$R/g_Fall_orange" game Fall 8 96 orange
run "$FBX" "$R/g_Stop_blue" game Stop 8 96 blue
run "$FBX" "$R/g_Check_orange" game Check 8 96 orange
cd "$R"
ff() { ffmpeg -y -loglevel error "$@"; }
PAL=",split[a][b];[a]palettegen=max_colors=96[p];[b][p]paletteuse=dither=none"
ff -i turn_rest/f%02d.png -vf tile=8x1 row_turn_rest.png
ff -i turn/f%02d.png -vf tile=8x1 row_turn.png
ff -i row_turn_rest.png -i row_turn.png -filter_complex "vstack=inputs=2$PAL" "$S/sheets/turntable.png"
for c in $CLIPS; do ff -i m_$c/f%02d.png -vf tile=8x1 row_$c.png; done
ff -i side_Stride/f%02d.png -vf tile=8x1 row_side_Stride.png
ff -i back_Stride/f%02d.png -vf tile=8x1 row_back_Stride.png
ff -i back_Crossover_Left/f%02d.png -vf tile=8x1 row_back_Crossover_Left.png
ff -i lean_L/f%02d.png -vf tile=4x1 lean_L.png
ff -i lean_R/f%02d.png -vf tile=4x1 lean_R.png
ff -i lean_L.png -i lean_R.png -filter_complex hstack=inputs=2 row_leans.png
ff -i row_Idle_Faceoff.png -i row_Stride.png -i row_side_Stride.png -i row_back_Stride.png -i row_Glide.png -i row_leans.png -i row_Stop.png -filter_complex vstack=inputs=7$PAL "$S/sheets/clips_skating.png"
ff -i row_Crossover_Left.png -i row_back_Crossover_Left.png -i row_Crossover_Right.png -filter_complex vstack=inputs=3$PAL "$S/sheets/clips_crossover.png"
ff -i row_Shot_Slap.png -i row_Shot_Wrist.png -i row_Check.png -filter_complex vstack=inputs=3$PAL "$S/sheets/clips_shots_check.png"
ff -i row_Fall.png -i row_Fall_Forward.png -i row_GetUp.png -i row_Celebrate.png -filter_complex vstack=inputs=4$PAL "$S/sheets/clips_fall_celebrate.png"
G="g_Stride_blue g_Stride_orange g_Shot_Slap_blue g_Fall_orange g_Stop_blue g_Check_orange"
for g in $G; do ff -i $g/f%02d.png -vf "tile=8x1" $g.png; done
ff -i g_Stride_blue.png -i g_Stride_orange.png -i g_Shot_Slap_blue.png -i g_Fall_orange.png -i g_Stop_blue.png -i g_Check_orange.png -filter_complex "vstack=inputs=6,scale=iw*2:ih*2:flags=neighbor$PAL" "$S/sheets/distance_40px.png"
ff -i row_Fall_Forward.png -i row_GetUp_Front.png -filter_complex vstack=inputs=2$PAL "$S/sheets/getup_front.png"
ls -la "$S/sheets"
