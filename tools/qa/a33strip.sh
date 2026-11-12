#!/usr/bin/env bash
# Lane A33: an 8-frame strip (every 3rd captured frame from hit-3 to hit+18) out of the PIE captures of tools/qa/tackle.py.
#   bash tools/qa/a33strip.sh <hit sim frame> <label> "<caption>"
# Reads Game/Saved/Screenshots/WindowsEditor/a33-win-f<frame>.png (falls back to the a33-clip- capture of the same frame), writes
# docs/experiments/img/tackling/<label>-strip.png through tools/meshy/strip.py (PIL).
set -e
HIT=$1; LABEL=$2; CAP=$3
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SHOTS="$ROOT/Game/Saved/Screenshots/WindowsEditor"
T="$(mktemp -d)"
i=0
for off in -3 0 3 6 9 12 15 18; do
  f=$((HIT + off))
  src="$SHOTS/$(printf 'a33-win-f%06d.png' $f)"
  [ -f "$src" ] || src="$SHOTS/$(printf 'a33-clip-f%06d.png' $f)"
  [ -f "$src" ] || src="$SHOTS/$(printf 'a33-win-f%06d.png' $((f + 1)))"
  [ -f "$src" ] || { echo "missing frame $f"; continue; }
  cp "$src" "$T/f0$i.png"; i=$((i + 1))
done
mkdir -p "$ROOT/docs/experiments/img/tackling"
python -I "$ROOT/tools/meshy/strip.py" "$T" "$ROOT/docs/experiments/img/tackling/$LABEL-strip.png" 8 600 "$CAP"
rm -rf "$T"
