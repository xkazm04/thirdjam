#!/usr/bin/env bash
# Lane A45: an 8-frame strip (every 3rd captured frame from hit-3 to hit+18) out of the PIE captures of tools/qa/v4.py.
#   bash tools/qa/a45strip.sh <sim frame> <label> "<caption>" [capture tag: win | pin]
# Reads Game/Saved/Screenshots/WindowsEditor/a45-<tag>-f<frame>.png (the other tag, or the next frame, as fallbacks), writes
# docs/experiments/img/tackling/<label>-strip.png through tools/meshy/strip.py (PIL).
set -e
HIT=$1; LABEL=$2; CAP=$3; TAG=${4:-win}
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SHOTS="$ROOT/Game/Saved/Screenshots/WindowsEditor"
T="$(mktemp -d)"
i=0
for off in -3 0 3 6 9 12 15 18; do
  f=$((HIT + off))
  src=""
  for cand in "$TAG" win pin; do
    for ff in $f $((f + 1)) $((f - 1)); do
      c="$SHOTS/$(printf 'a45-%s-f%06d.png' "$cand" $ff)"
      if [ -f "$c" ]; then src="$c"; break 2; fi
    done
  done
  if [ -z "$src" ]; then echo "missing frame $f"; continue; fi
  cp "$src" "$T/f0$i.png"; i=$((i + 1))
done
mkdir -p "$ROOT/docs/experiments/img/tackling"
python -I "$ROOT/tools/meshy/strip.py" "$T" "$ROOT/docs/experiments/img/tackling/$LABEL-strip.png" 8 600 "$CAP"
rm -rf "$T"
