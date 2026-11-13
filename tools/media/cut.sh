#!/bin/bash
# Lane A88: cut submission/video/hokej-v1.mp4 from the takes (1920x1080, 30 fps, silent AAC track so players accept it; real audio is added later).
# Each line: take|start|duration|caption. Run from the worktree root. Captions are plain text, no brand words.
set -e
T=${TAKES:-submission/video/takes}; O=submission/video; W=tmp_cut; mkdir -p $W; rm -f $W/*
FONT='C\:/Windows/Fonts/arialbd.ttf'
i=0
while IFS='|' read -r take ss dur cap; do
  [ -z "$take" ] && continue; i=$((i+1)); n=$(printf %02d $i)
  vf="scale=1920:1080,fps=30,format=yuv420p"
  if [ -n "$cap" ]; then vf="$vf,drawtext=fontfile='$FONT':text='$cap':x=60:y=h-215:fontsize=64:fontcolor=white:borderw=4:bordercolor=black"; fi
  ffmpeg -nostdin -v error -y -ss $ss -t $dur -i $T/$take/take.mp4 -vf "$vf" -an -c:v libx264 -crf 17 -preset medium $W/c$n.mp4
  echo "file 'c$n.mp4'" >> $W/list.txt
done <<'LIST'
menus-H24|2.0|2.0|HOKEJ
menus-H24|5.0|4.0|12 MODES. YOU PICK THE PAIN.
menus-H24|14.0|3.5|FACEOFF
dev5-check|55.0|7.0|BODY CHECK
dev5-check|90.0|8.0|DOWN. GET UP ANGRY.
long1-bruisers|58.0|6.0|CLAY AND INK
menus-H24|28.5|3.0|BLUE SCORES
menus-H24|36.0|4.0|THE DIRECTOR CHANGES THE RULES
LIST
ffmpeg -v error -y -f lavfi -i "color=c=black:s=1920x1080:r=30:d=2" -vf "drawtext=fontfile='$FONT':text='HOKEJ':x=(w-text_w)/2:y=(h-text_h)/2-40:fontsize=200:fontcolor=white,drawtext=fontfile='$FONT':text='RAW ICE. CLAY BODIES.':x=(w-text_w)/2:y=(h/2)+110:fontsize=64:fontcolor=0xE8651A,format=yuv420p" -c:v libx264 -crf 17 $W/c99.mp4
echo "file 'c99.mp4'" >> $W/list.txt
ffmpeg -v error -y -f concat -safe 0 -i $W/list.txt -f lavfi -i anullsrc=r=48000:cl=stereo -shortest -c:v libx264 -crf 18 -preset slow -pix_fmt yuv420p -r 30 -c:a aac -b:a 128k -movflags +faststart $O/hokej-v1.mp4
rm -rf $W
ffprobe -v error -show_entries stream=codec_name,width,height,r_frame_rate:format=duration,size -of default=nw=1 $O/hokej-v1.mp4
