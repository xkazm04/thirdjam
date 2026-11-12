#!/usr/bin/env bash
# Sheets for one candidate, all from its re-imported export. Usage: bash render_cand.sh <name>
# Writes sheets/<name>_motion.png (stride 3/4, side, behind; glide 3/4; slap side and 3/4), sheets/<name>_holds.png
# (turntable rest; goalie hold front and side; 40 px strips from behind: Stride, Shot_Slap, Glide, Goalie_Hold).
set -e
N=$1
B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"
S=C:/Users/kazda/jam/skater-lab/skh
F=$S/export/$N/SK_Skater_$N.fbx
R=$S/r/$N; SH=$S/sheets
mkdir -p $R $SH
row() {
  local name=$1 mode=$2 act=$3; shift 3
  local aa=(); [ "$act" != "-" ] && aa=(--action "$act")
  "$B" -b --factory-startup --python $S/render_skh.py -- --in "$F" --out $R/$name --mode $mode --zoom 0.85 --tz 0.72 "${aa[@]}" "$@" > $R/$name.log 2>&1
  local n=$(ls $R/$name/f*.png | wc -l)
  ffmpeg -y -loglevel error -i $R/$name/f%02d.png -vf "tile=${n}x1" $R/$name.png
}
row tt    turntable -          --n 8 &
row st_34 front34   Stride &
row st_sd side      Stride &
row st_bk back      Stride &
wait
row gl_34 front34   Glide &
row sl_sd side      Shot_Slap &
row sl_34 front34   Shot_Slap &
row gh_fr front     Goalie_Hold --n 4 &
row gh_sd side      Goalie_Hold --n 4 &
wait
row g_st  game Stride      --size 96 --n 8 &
row g_sl  game Shot_Slap   --size 96 --n 8 &
row g_gl  game Glide       --size 96 --n 4 &
row g_gh  game Goalie_Hold --size 96 --n 4 &
wait
pal="split[x][y];[x]palettegen=max_colors=192[p];[y][p]paletteuse=dither=none"
ffmpeg -y -loglevel error -i $R/st_34.png -i $R/st_sd.png -i $R/st_bk.png -i $R/gl_34.png -i $R/sl_sd.png -i $R/sl_34.png \
  -filter_complex "vstack=inputs=6,$pal" $SH/${N}_motion.png
ffmpeg -y -loglevel error -i $R/gh_fr.png -i $R/gh_sd.png -filter_complex "hstack,$pal" $R/gh.png
ffmpeg -y -loglevel error -i $R/g_st.png -i $R/g_sl.png -filter_complex "hstack,scale=iw*2:ih*2:flags=neighbor" $R/g1.png
ffmpeg -y -loglevel error -i $R/g_gl.png -i $R/g_gh.png -filter_complex "hstack,scale=iw*2:ih*2:flags=neighbor" $R/g2.png
ffmpeg -y -loglevel error -i $R/tt.png -i $R/gh.png -i $R/g1.png -i $R/g2.png \
  -filter_complex "[0]scale=1536:-1[a];[1]scale=1536:-1[b];[2]scale=1536:-1:flags=neighbor[c];[3]scale=1536:-1:flags=neighbor[d];[a][b][c][d]vstack=inputs=4,$pal" $SH/${N}_holds.png
ls -la $SH/${N}_*.png
