#!/usr/bin/env bash
# SKG: goalie kit + goalie clips + exports + checks + sheets, one command.
#   bash C:/Users/kazda/jam/skater-lab/skg/run_all.sh [style|all] [--skeleton <fbx or blend>] [--no-render]
# style: p2_cel | p3_clay | p4_broadcast | all (default all).
# Skeleton: --skeleton wins; otherwise p2_cel uses the s4 figure (rebuilt here into base/ from s4's build_skater.py copy), and
# p3_clay / p4_broadcast use lane SKF's figure skf/export/<style>/SK_Skater_<style>.fbx when it exists, else the s4 figure.
# The s4-skeleton goalie takes (export/SK_Goalie_clips_loopcut.fbx) come from the p2_cel run on the s4 figure.
set -e
S="$(cd "$(dirname "$0")" && pwd -W 2>/dev/null || pwd)"
B="/c/Program Files/Blender Foundation/Blender 4.2/blender.exe"
STYLE=all; SKEL=""; RENDER=1
while [ $# -gt 0 ]; do
  case "$1" in
    --skeleton) SKEL="$2"; shift 2;;
    --no-render) RENDER=0; shift;;
    *) STYLE="$1"; shift;;
  esac
done
[ "$STYLE" = all ] && STYLES="p2_cel p3_clay p4_broadcast" || STYLES="$STYLE"
bl() { "$B" -b --factory-startup --python "$@" 2>&1 | grep -vE "TBBmalloc|^$"; }
mkdir -p "$S/base" "$S/export" "$S/logs"
bl "$S/build_skater.py" -- "$S/base" | grep -E "S4 BUILD|rror" || true
for st in $STYLES; do
  sk="$SKEL"
  if [ -z "$sk" ]; then
    sk="$S/base/s4_skater.blend"
    f="$S/../skf/export/$st/SK_Skater_$st.fbx"
    if [ "$st" != p2_cel ] && [ -f "$f" ]; then sk="$f"; fi
  fi
  echo "== $st on $sk"
  bl "$S/build_goalie.py" -- "$S" "$st" "$sk" --export-skel > "$S/logs/build_$st.log"
  grep -E "SKG KIT |SKG FIT|EXPORT OK|rror|Traceback" "$S/logs/build_$st.log" || true
  echo "$sk" > "$S/export/$st/skeleton_used.txt"
  bl "$S/check_fbx.py" -- "$S/export/$st/SK_Goalie_clips_loopcut.fbx" "$S/export/$st/check_fbx_loopcut.json" | grep -E "rror" || true
  bl "$S/check_fbx.py" -- "$S/export/$st/SK_Goalie_clips.fbx" "$S/export/$st/check_fbx.json" | grep -E "rror" || true
  bl "$S/check_kit.py" -- "$S/export/$st/SK_Goalie_clips.fbx" "$S/export/$st" "$S/export/$st/check_kit.json" | grep -E "SKGKIT|rror" || true
  if [ "$st" = p2_cel ] && [ -z "$SKEL" ]; then
    cp "$S/export/p2_cel/SK_Goalie_clips_loopcut.fbx" "$S/export/p2_cel/SK_Goalie_clips.fbx" "$S/export/p2_cel/attach.json" "$S/export/"
    cp "$S/export/p2_cel/check_fbx_loopcut.json" "$S/export/check_fbx_loopcut.json"
  fi
  [ $RENDER = 1 ] && bash "$S/render_sheets.sh" "$st"
done
ls -la "$S/export" "$S/sheets" 2>/dev/null | grep -vE "^total"
