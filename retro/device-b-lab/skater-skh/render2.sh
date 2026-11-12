#!/usr/bin/env bash
# Second-pass sheets for one export2 folder: bash render2.sh <folder> <asset>
# sheets/2_<folder>.png: Stride 3/4, side, behind; Shot_Slap side; Goalie_Hold front + side;
# then the four aim holds and the Poke extreme (frame 10) from the side, zoomed out so the whole stick is in frame.
set -e
N=$1; A=$2
B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"
S=C:/Users/kazda/jam/skater-lab/skh
F=$S/export2/$N/SK_Skater_$A.fbx
R=$S/r2/$N; SH=$S/sheets
mkdir -p $R $SH
row() {
  local name=$1 mode=$2 act=$3; shift 3
  "$B" -b --factory-startup --python $S/render_skh.py -- --in "$F" --out $R/$name --mode $mode --action $act "$@" > $R/$name.log 2>&1
  local n=$(ls $R/$name/f*.png | wc -l)
  ffmpeg -y -loglevel error -i $R/$name/f%02d.png -vf "tile=${n}x1" $R/$name.png
}
Z="--zoom 0.85 --tz 0.72"
row st_34 front34 Stride $Z & row st_sd side Stride $Z & row st_bk back Stride $Z & row sl_sd side Shot_Slap $Z & wait
row gh_fr front Goalie_Hold $Z --n 4 & row gh_sd side Goalie_Hold $Z --n 4 &
W="--zoom 0.45 --tz 0.6 --size 307"
for a in Aim_Front Aim_Left Aim_Right Aim_FarReach; do row r_$a side $a $W --frames 1 & done
row r_Poke side Poke $W --frames 10 &
wait
pal="split[x][y];[x]palettegen=max_colors=192[p];[y][p]paletteuse=dither=none"
ffmpeg -y -loglevel error -i $R/gh_fr.png -i $R/gh_sd.png -filter_complex "hstack" $R/gh.png
ffmpeg -y -loglevel error -i $R/r_Aim_Front.png -i $R/r_Aim_Left.png -i $R/r_Aim_Right.png -i $R/r_Aim_FarReach.png -i $R/r_Poke.png -filter_complex "hstack=inputs=5,scale=1536:-1" $R/reach.png
ffmpeg -y -loglevel error -i $R/st_34.png -i $R/st_sd.png -i $R/st_bk.png -i $R/sl_sd.png -i $R/gh.png -i $R/reach.png -filter_complex "vstack=inputs=6,$pal" $SH/2_$N.png
ls -la $SH/2_$N.png
