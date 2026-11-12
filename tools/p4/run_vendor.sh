#!/usr/bin/env bash
# tools/p4/run_vendor.sh (lane A28): one vendor figure through the editor on MCP port 8015, start to contact sheet.
#   tools/p4/run_vendor.sh <Vendor> [clip_asset_path] [tag]
# Steps: delete /Game/P4/<Vendor> (clean re-import), ue_setup.py (import, measure, retarget from Manny, physics asset), physics asset from the
# PhysicsAssetToolset (the subsystem one gives two bodies), ue_lab.py (lab level, Manny beside the figure, cameras), Simulate + ue_capture.py
# (2 s of the clip, then 20 frames of ragdoll and back), sheet.py into devlog/shots/. Needs the editor started by this lane on port 8015.
set -u
export MSYS_NO_PATHCONV=1
cd "$(dirname "$0")/../.."
V="$1"; CLIP="${2:-/Game/P4/$V/Anims/MF_Unarmed_Walk_Fwd_P4_$V}"; TAG="${3:-$(echo "$V" | tr 'A-Z' 'a-z')-walk}"
U="--url http://127.0.0.1:8015/mcp"; M="node tools/mcp/mcp.mjs"; PY="editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python"
L=$(echo "$V" | tr 'A-Z' 'a-z'); OUT="C:/Users/kazda/kiro/thirdjam/tools/p4/out/$L/cap"; J="tools/p4/jobs"
mkdir -p "$OUT"; rm -f "$OUT"/*.png
if [ -z "${CAPTURE_ONLY:-}" ]; then
echo "A28 run $V start $(date +%H:%M:%S)"
cat > "$J/_${L}_delete.mcp.json" <<EOF
{"script":{"content":"import unreal\nlib=unreal.EditorAssetLibrary\nprint('A28 $V delete', lib.delete_directory('/Game/P4/$V') if lib.does_directory_exist('/Game/P4/$V') else 'absent')\n","language":"Python"}}
EOF
$M call $PY @"$J/_${L}_delete.mcp.json" $U --timeout 120 | tail -1
cat > "$J/_${L}_setup.mcp.json" <<EOF
{"script":{"content":"A28_CFG=r'C:/Users/kazda/kiro/thirdjam/tools/p4/jobs/$L.ue.json'\nexec(open(r'C:/Users/kazda/kiro/thirdjam/tools/p4/ue_setup.py').read())\n","language":"Python"}}
EOF
$M call $PY @"$J/_${L}_setup.mcp.json" $U --timeout 900 | tail -1
$M call PhysicsToolsets.PhysicsAssetToolset.CreateFromMesh "{\"meshPath\":\"/Game/P4/$V/SK_P4_$V\",\"bAssignToMesh\":true}" $U --timeout 120 | tail -1
cat > "$J/_${L}_pa.mcp.json" <<EOF
{"script":{"content":"import unreal\nlib=unreal.EditorAssetLibrary\nD='/Game/P4/$V'\nold=D+'/PA_P4_$V'\nif lib.does_asset_exist(D+'/SK_P4_${V}_PhysicsAsset'):\n    if lib.does_asset_exist(old): lib.delete_asset(old)\n    print('A28 $V pa rename', lib.rename_asset(D+'/SK_P4_${V}_PhysicsAsset', old))\npa=lib.load_asset(old); mesh=lib.load_asset(D+'/SK_P4_$V')\nmesh.set_editor_property('physics_asset', pa)\nlib.save_asset(D+'/SK_P4_$V', only_if_is_dirty=False); lib.save_asset(old, only_if_is_dirty=False)\nprint('A28 $V pa assigned', pa.get_path_name())\n","language":"Python"}}
EOF
$M call $PY @"$J/_${L}_pa.mcp.json" $U --timeout 120 | tail -1
$M call PhysicsToolsets.PhysicsAssetToolset.GetBodyNames "{\"physicsAsset\":{\"refPath\":\"/Game/P4/$V/PA_P4_$V.PA_P4_$V\"}}" $U --timeout 60 | grep -v '^{"tool"' | tr -d '\n ' | cut -c1-400; echo
cat > "$J/_${L}_lab.mcp.json" <<EOF
{"script":{"content":"A28_LAB={'vendor':'$V','mesh':'/Game/P4/$V/SK_P4_$V'}\nexec(open(r'C:/Users/kazda/kiro/thirdjam/tools/p4/ue_lab.py').read())\n","language":"Python"}}
EOF
$M call $PY @"$J/_${L}_lab.mcp.json" $U --timeout 300 | tail -1
fi
python -I - "$V" "$CLIP" "$OUT" "$TAG" "$J/_${L}_capture.mcp.json" <<'EOF'
import json, sys
V, CLIP, OUT, TAG, dst = sys.argv[1:6]
s = open("tools/p4/ue_capture.py", encoding="utf8").read()
s = s.replace("__VENDOR__", V).replace("__CLIP__", CLIP).replace("__OUT__", OUT).replace("__TAG__", TAG).replace("__RAGDOLL__", "1")
json.dump({"script": {"content": s, "language": "Python"}}, open(dst, "w"))
EOF
$M call EditorToolset.EditorAppToolset.StartPIE '{"options":{"bSimulate":true}}' $U --timeout 120 | tail -1
sleep 4
$M call $PY @"$J/_${L}_capture.mcp.json" $U --timeout 300 | tail -1 | cut -c1-300
$M call EditorToolset.EditorAppToolset.StopPIE '{}' $U --timeout 60 | tail -1
grep -a "A28 $V " Game/logs/a28-editor.log | tail -40 | sed 's/^.*LogPython: //' | cut -c1-600
TS=$(date +%Y%m%d-%H%M%S); mkdir -p devlog/shots
python -I tools/p4/sheet.py "$OUT" "$TAG" chase "devlog/shots/$TS-A-a28-$TAG-chase.png" "devlog/shots/$TS-A-a28-$TAG-chase.mp4" "A28 $V $TAG chase 4.2 m: f00-f15 = 2 s of the clip (8 game frames apart), rag00-05 = ragdoll 20 frames, back00-03 = sequence again"
python -I tools/p4/sheet.py "$OUT" "$TAG" far "devlog/shots/$TS-A-a28-$TAG-far.png" - "A28 $V $TAG far 24 m"
echo "A28 run $V end $(date +%H:%M:%S)"
