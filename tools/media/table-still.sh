#!/usr/bin/env bash
# Render a text file to a 1280x720 still with ffmpeg drawtext (lane A96). No game window, no browser.
# usage: bash tools/media/table-still.sh <in.txt> <out.png>
set -e
FF=${FFMPEG:-/c/ffmpeg/bin/ffmpeg.exe}
IN=$(cd "$(dirname "$1")" && pwd)/$(basename "$1"); OUT=$2
cd "$(dirname "$IN")"
"$FF" -y -loglevel error -f lavfi -i "color=c=0x14161c:s=1280x720" -vf "drawtext=fontfile='C\:/Windows/Fonts/consola.ttf':textfile='$(basename "$IN")':fontcolor=0xe8e4d8:fontsize=22:line_spacing=3:x=50:y=24" -frames:v 1 "$OUT"
