#!/usr/bin/env bash
# One command for the whole S4 lane: build mesh + rig, author clips + export, inspect, check, render, contact sheets.
#   bash C:/Users/kazda/jam/skater-lab/s4/run_all.sh
set -e
S="$(cd "$(dirname "$0")" && pwd -W 2>/dev/null || pwd)"
B="/c/Program Files/Blender Foundation/Blender 4.2/blender.exe"
INSPECT="C:/Users/kazda/jam/thirdjam/tools/meshy/inspect.py"
bl() { "$B" -b --factory-startup --python "$@" 2>&1 | grep -vE "TBBmalloc|^$"; }
time bl "$S/build_skater.py" -- "$S" | grep -E "S4|rror"
time bl "$S/build_clips.py" -- "$S" | grep -E "S4 EXPORT|rror"
# DISABLED 12:35 by the lead: Unreal 5.9 FBX importer rejects the rewritten file ("error when parsing the file"); root now has identity rest, so its identity keys are harmless. for f in SK_Skater_S4 SK_Skater_S4_loopcut; do bl "$S/strip_tracks.py" -- "$S/export/$f.fbx" | grep -E "STRIP|rror"; done
bl "$INSPECT" -- "$S/export/SK_Skater_S4.fbx" "$S/export/SK_Skater_S4_loopcut.fbx" "$S/export/SK_Skater_S4.glb" | grep -E '^\{' > "$S/inspect.jsonl"
bl "$S/check_fbx.py" -- "$S/export/SK_Skater_S4.fbx" "$S/check_fbx.json" | grep -E "rror" || true
bl "$S/check_fbx.py" -- "$S/export/SK_Skater_S4_loopcut.fbx" "$S/check_fbx_loopcut.json" | grep -E "rror" || true
for f in SK_Skater_S4 SK_Skater_S4_loopcut; do bl "$S/fbx_nodes.py" -- "$S/export/$f.fbx" | grep -E "GLOBAL|MODEL"; bl "$S/fbx_tracks.py" -- "$S/export/$f.fbx" | grep TRACKS; done > "$S/fbx_nodes.txt"
time bash "$S/render_all.sh"
