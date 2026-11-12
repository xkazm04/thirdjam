#!/usr/bin/env bash
# tools/blender/run_all.sh: the whole A22 Blender job list, headless. Run from the repository root in Git Bash:
#   bash tools/blender/run_all.sh            # figures (3 variants, FBX + renders), props, contact sheet, turntables
#   bash tools/blender/run_all.sh figures    # only the figures
#   bash tools/blender/run_all.sh props      # only the props
#   bash tools/blender/run_all.sh sheet      # only the ffmpeg steps (sheet + turntable MP4s) from existing renders
# Needs: Blender 4.2 (BLENDER env or the default install path), ffmpeg on PATH. Blender renders on the GPU (EEVEE):
# stop this script when the Director asks for the GPU. Never runs git. Timings go to tools/blender/out/timings.txt.
set -u
BLENDER="${BLENDER:-/c/Program Files/Blender Foundation/Blender 4.2/blender.exe}"
OUT=tools/blender/out
IMG=docs/experiments/img
mkdir -p "$OUT/render" "$IMG"
STEP="${1:-all}"
log() { echo "$(date '+%Y-%m-%d %H:%M:%S') $*" | tee -a "$OUT/timings.txt"; }

run_blender() {  # name script args...
  local name=$1; shift
  local t0=$(date +%s)
  "$BLENDER" -b -P "$@" > "$OUT/log_$name.txt" 2>&1
  local rc=$?
  local t1=$(date +%s)
  if grep -q "A22 .* PASS" "$OUT/log_$name.txt"; then
    log "PASS $name $((t1-t0)) s"
  else
    log "FAIL $name rc=$rc $((t1-t0)) s (see $OUT/log_$name.txt)"; grep -E "Error|Traceback|line [0-9]+" "$OUT/log_$name.txt" | head -5
  fi
}

if [ "$STEP" = all ] || [ "$STEP" = figures ]; then
  for v in toy clay broadcast; do
    run_blender "skater_$v" tools/blender/skater.py -- --variant $v --out "$OUT" --render --turntable
  done
fi
if [ "$STEP" = all ] || [ "$STEP" = props ]; then
  run_blender props tools/blender/props.py -- --out "$OUT" --render
fi
if [ "$STEP" = all ] || [ "$STEP" = sheet ] || [ "$STEP" = figures ]; then
  # 3 x 3 contact sheet: rows toy, clay, broadcast; columns idle, carry, poke; cells 1024 px
  FONT="C\:/Windows/Fonts/arial.ttf"
  LABELS=""
  r=0; for v in toy clay broadcast; do c=0; for pose in "idle f0" "carry f15" "poke f6"; do
    LABELS="$LABELS,drawtext=fontfile='$FONT':text='$v / $pose':x=$((c*1024+24)):y=$((r*1024+24)):fontsize=40:fontcolor=white:box=1:boxcolor=black@0.45:boxborderw=10"
    c=$((c+1)); done; r=$((r+1)); done
  ffmpeg -y -loglevel error -framerate 1 -i "$OUT/render/cell_%02d.png" -frames:v 1 -vf "tile=3x3$LABELS" "$IMG/blender-sheet.png" && log "PASS sheet $IMG/blender-sheet.png $(stat -c %s "$IMG/blender-sheet.png") bytes"
  ffmpeg -y -loglevel error -i "$IMG/blender-sheet.png" -vf "scale=1536:-1" "$IMG/blender-sheet-1536.png"
  for v in toy clay broadcast; do
    if [ -f "$OUT/render/${v}_tt_000.png" ]; then
      ffmpeg -y -loglevel error -framerate 30 -i "$OUT/render/${v}_tt_%03d.png" -vf "drawtext=fontfile='$FONT':text='$v':x=16:y=16:fontsize=24:fontcolor=white:box=1:boxcolor=black@0.45:boxborderw=6" -c:v libx264 -pix_fmt yuv420p -crf 24 -movflags +faststart "$IMG/blender-turntable-$v.mp4" \
        && log "PASS turntable $v $(stat -c %s "$IMG/blender-turntable-$v.mp4") bytes"
    fi
  done
fi
if [ "$STEP" = all ] || [ "$STEP" = props ]; then
  [ -f "$OUT/render/props_overview.png" ] && cp "$OUT/render/props_overview.png" "$IMG/blender-props.png" && log "PASS props overview $IMG/blender-props.png"
fi
log "done ($STEP)"
