#!/usr/bin/env bash
# bash render3.sh <folder> <asset> [stride]: Fall, Fall_Forward, GetUp, GetUp_Front from the side (+ Stride 3 views if "stride")
N=$1; A=$2; X=$3
B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"; S=C:/Users/kazda/jam/skater-lab/skh
F=$S/export3/$N/SK_Skater_$A.fbx; R=$S/r3/$N; mkdir -p $R
row() { local name=$1 mode=$2 act=$3; "$B" -b --factory-startup --python $S/render_skh.py -- --in "$F" --out $R/$name --mode $mode --action $act --zoom 0.85 --tz 0.6 > $R/$name.log 2>&1; ffmpeg -y -loglevel error -i $R/$name/f%02d.png -vf tile=8x1 $R/$name.png; }
for a in Fall Fall_Forward GetUp GetUp_Front; do row $a side $a & done
if [ "$X" = stride ]; then row s34 front34 Stride & row ssd side Stride & row sbk back Stride & fi
wait
pal="split[x][y];[x]palettegen=max_colors=128[p];[y][p]paletteuse=dither=none"
ffmpeg -y -loglevel error -i $R/Fall.png -i $R/Fall_Forward.png -i $R/GetUp.png -i $R/GetUp_Front.png -filter_complex "vstack=inputs=4,$pal" $S/sheets/3_${N}_falls.png
[ "$X" = stride ] && ffmpeg -y -loglevel error -i $R/s34.png -i $R/ssd.png -i $R/sbk.png -filter_complex "vstack=inputs=3,$pal" $S/sheets/3_${N}_stride.png
ls -la $S/sheets/3_${N}_*.png
