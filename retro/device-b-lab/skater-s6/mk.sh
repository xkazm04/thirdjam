B="C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"; D=C:/Users/kazda/jam/skater-lab/s6
cd $D
"$B" -b -P clean_texture.py 2>&1 | grep -E "masked|Error|line"; "$B" -b -P verify.py 2>&1 | grep -E "changed"
"$B" -b -P render.py -- $D/T_SkaterTB2_BaseColor_clean.png $D/r_after after close 2>&1 | grep -i -E "Traceback"
cd r_after; ffmpeg -y -loglevel error -i after_shin000.png -i after_shin045.png -i after_shin090.png -i after_shin135.png -i after_shin180.png -i after_shin225.png -i after_shin270.png -i after_shin315.png -filter_complex "xstack=inputs=8:layout=0_0|w0_0|w0+w1_0|w0+w1+w2_0|0_h0|w0_h0|w0+w1_h0|w0+w1+w2_h0" ../sheet_a.png
